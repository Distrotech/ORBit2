#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linc/linc.h>
#include "linc-private.h"

#define SYS_SOCKET_BUFFER_MAX (512 * 1024)
#define BUFFER_MAX 1024

static void
test_protos (void)
{
	LINCProtocolInfo *info;

	info = linc_protocol_all ();

	fprintf (stderr, "Available protocols: {\n");

	while (info && info->name) {
		fprintf (stderr, "\t'%8s': %2d, %3d, %2d, 0x%.4x [%c%c%c%c%c]\n",
			 info->name, info->family, info->addr_len,
			 info->stream_proto_num, info->flags,
			 info->setup ?        's' : '-',
			 info->destroy ?      'd' : '-',
			 info->get_sockaddr ? 'a' : '-',
			 info->get_sockinfo ? 'i' : '-',
			 info->is_local ?     'l' : '-');
		info++;
	}
	
	fprintf (stderr, " }\n");
}

static void
init_tmp (void)
{
	char *dir;
	const char *user = g_getenv ("USER");

	dir = g_strconcat ("/tmp/orbit-", user, NULL);

	linc_set_tmpdir (dir);

	g_free (dir);
}

static void
broken_cb (LINCConnection *cnx, gpointer user_data)
{
	g_assert (user_data == NULL);

	exit (13);
}

GType    test_server_cnx_type = 0;
GType    test_client_cnx_type = 0;
gboolean connected = FALSE;

static LINCConnection *
test_server_create_connection (LINCServer *cnx)
{
	GType t;

	t = test_server_cnx_type ? test_server_cnx_type : linc_connection_get_type ();

	connected = TRUE;

	return g_object_new (t, NULL);
}

static void
create_server (LINCServer **server)
{
	LINCServerClass *klass;

	klass = g_type_class_ref (linc_server_get_type ());
	klass->create_connection = test_server_create_connection;

	*server = g_object_new (linc_server_get_type (), NULL);
	
	g_assert (linc_server_setup (*server, "UNIX", NULL, NULL,
				     LINC_CONNECTION_NONBLOCKING));

	g_object_add_weak_pointer (G_OBJECT (*server),
				   (gpointer *) server);
}

static void
create_client (LINCServer *server, LINCConnection **client)
{
	*client = g_object_new (
			test_client_cnx_type ? test_client_cnx_type :
					       linc_connection_get_type (),
			NULL);
	
	g_assert (linc_connection_initiate (
		*client, "UNIX",
		server->local_host_info,
		server->local_serv_info,
		LINC_CONNECTION_NONBLOCKING));

	g_object_add_weak_pointer (G_OBJECT (*client),
				   (gpointer *) client);
}

static gboolean 
test_broken_cnx_handle_input (LINCConnection *cnx)
{
	glong  ret;
	guchar buffer;

	ret = linc_connection_read (cnx, &buffer, 1, FALSE);

	g_assert (ret == LINC_IO_FATAL_ERROR);

	return TRUE;
}

static void
test_broken_cnx_class_init (LINCConnectionClass *klass)
{
	klass->handle_input = test_broken_cnx_handle_input;
}

static GType
test_get_broken_cnx_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (LINCConnectionClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) test_broken_cnx_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (LINCConnection),
			0,              /* n_preallocs */
			(GInstanceInitFunc) NULL,
		};
      
		object_type = g_type_register_static (
			LINC_TYPE_CONNECTION, "TestConnection",
			&object_info, 0);
	}

	return object_type;
}

static void
test_broken (void)
{
	LINCServer     *server;
	LINCConnection *client;
	pid_t           child;
	int             status;

	fprintf (stderr, "Testing 'broken' ...\n");

	create_server (&server);

	if ((child = fork ()) == 0) { /* child */
		test_client_cnx_type = test_get_broken_cnx_type ();
		create_client (server, &client);
		test_client_cnx_type = 0;

		g_signal_connect (G_OBJECT (client), "broken",
				  G_CALLBACK (broken_cb), NULL);

		g_object_unref (G_OBJECT (server));
		g_assert (server == NULL);

		linc_main_loop_run ();

		g_assert_not_reached ();
	}

	while (!connected)
		linc_main_iteration (FALSE);
	connected = FALSE;

	g_object_unref (G_OBJECT (server));
	g_assert (server == NULL);

	waitpid (child, &status, 0);
	g_assert (WIFEXITED (status) && WEXITSTATUS (status) == 13);
}

static GIOCondition
knobble_watch (LincWatch *watch, GIOCondition new_cond)
{
	GIOCondition   old_cond;
	LincUnixWatch *a = (LincUnixWatch *) watch->linc_source;
	LincUnixWatch *b = (LincUnixWatch *) watch->main_source;

	g_assert (watch != NULL);

	g_assert ((old_cond = a->condition) == b->condition);
	
	linc_watch_set_condition (watch, new_cond);

	return old_cond;
}

typedef struct {
	int             status;
	GIOCondition    old_cond;
	LINCConnection *s_cnx;
} BlockingData;

static void
blocking_cb (LINCConnection *cnx,
	     gulong          buffer_size,
	     gpointer        user_data)
{
	BlockingData *bd = user_data;

	if (bd->status < 3)
		fprintf (stderr, " buffer %ld\n", buffer_size);

	bd->status++;

	if (buffer_size == BUFFER_MAX) {
		knobble_watch (bd->s_cnx->priv->tag, bd->old_cond);

		/* flush the queue to other side */
		while (cnx->priv->write_queue != NULL &&
		       cnx->status == LINC_CONNECTED)
			linc_main_iteration (FALSE);

		g_assert (cnx->status == LINC_CONNECTED);
	}
}

static gboolean 
test_blocking_cnx_handle_input (LINCConnection *cnx)
{
	static gulong idx = 0;
	glong  size, i;
	glong  buffer[1024];

	size = linc_connection_read (cnx, (guchar *) buffer, 512, TRUE);
	g_assert (size != -1);
	g_assert ((size & 0x3) == 0);
	g_assert (size <= 512);

	for (i = 0; i < (size >> 2); i++)
		g_assert (buffer [i] == idx++);

	return TRUE;
}

static void
test_blocking_cnx_class_init (LINCConnectionClass *klass)
{
	klass->handle_input = test_blocking_cnx_handle_input;
}

static GType
test_get_blocking_cnx_type (void)
{
	static GType object_type = 0;

	if (!object_type) {
		static const GTypeInfo object_info = {
			sizeof (LINCConnectionClass),
			(GBaseInitFunc) NULL,
			(GBaseFinalizeFunc) NULL,
			(GClassInitFunc) test_blocking_cnx_class_init,
			NULL,           /* class_finalize */
			NULL,           /* class_data */
			sizeof (LINCConnection),
			0,              /* n_preallocs */
			(GInstanceInitFunc) NULL,
		};
      
		object_type = g_type_register_static (
			LINC_TYPE_CONNECTION, "TestConnection",
			&object_info, 0);
	}

	return object_type;
}

static void
test_blocking (void)
{
	BlockingData    bd;
	LINCServer     *server;
	LINCConnection *client;
	LINCWriteOpts  *options;
	glong           l, buffer[1024] = { 0 };
	int             i;

	fprintf (stderr, "Testing blocking code ...\n");

	/* Create our own LincConnection to verify input */
	test_server_cnx_type = test_get_blocking_cnx_type ();

	create_server (&server);
	create_client (server, &client);
	linc_main_iteration (FALSE); /* connect */

	g_assert (server->priv->connections != NULL);
	bd.s_cnx = server->priv->connections->data;
	g_assert (bd.s_cnx != NULL);
	g_assert (bd.s_cnx->priv->tag != NULL);
	bd.old_cond = knobble_watch (bd.s_cnx->priv->tag, 0); /* stop it listening */

	options = linc_write_options_new (FALSE);
	linc_connection_set_max_buffer (client, BUFFER_MAX);
	g_signal_connect (G_OBJECT (client), "blocking",
			  G_CALLBACK (blocking_cb), &bd);
	client->options |= LINC_CONNECTION_BLOCK_SIGNAL;

	l = 0;
	bd.status = 0;
	for (i = 0; i < SYS_SOCKET_BUFFER_MAX; i+= 128) {
		int j;

		for (j = 0; j < 128/4; j++)
			buffer [j] = l++;

		linc_connection_write (
			client, (guchar *) buffer, 128, options);
		if (client->status != LINC_CONNECTED)
			break;
	}

	g_assert (client->status == LINC_CONNECTED);
	g_assert (bd.status >= 3);

	g_object_unref (G_OBJECT (client));
	g_assert (client == NULL);

	linc_main_iteration (FALSE);

	g_object_unref (G_OBJECT (server));
	g_assert (server == NULL);

	test_server_cnx_type = 0;

	linc_write_options_free (options);
}

int
main (int argc, char **argv)
{	
	linc_init (FALSE);
	init_tmp ();

	test_protos ();
	test_broken ();
	test_blocking ();

	fprintf (stderr, "All tests passed successfully\n");
	
	return 0;
}