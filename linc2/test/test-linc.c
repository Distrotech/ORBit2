#include <stdio.h>
#include "linc-private.h"

gboolean broken;

#define SYS_SOCKET_BUFFER_MAX (512 * 1024)
#define BUFFER_MAX 1024

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
	g_assert (user_data == &broken);

	broken = TRUE;
}

static void
create_server (LINCServer **server)
{
	*server = g_object_new (linc_server_get_type (), NULL);
	
	g_assert (linc_server_setup (*server, "UNIX", NULL, NULL,
				     LINC_CONNECTION_NONBLOCKING));

	g_object_add_weak_pointer (G_OBJECT (*server),
				   (gpointer *) server);
}

static void
create_client (LINCServer *server, LINCConnection **client)
{
	*client = g_object_new (linc_connection_get_type (), NULL);
	
	g_assert (linc_connection_initiate (
		*client, "UNIX",
		server->local_host_info,
		server->local_serv_info,
		LINC_CONNECTION_NONBLOCKING));

	g_object_add_weak_pointer (G_OBJECT (*client),
				   (gpointer *) client);
}

static void
test_broken (void)
{
	LINCServer     *server;
	LINCConnection *client;

	fprintf (stderr, "Testing 'broken' ...\n");

	create_server (&server);

	create_client (server, &client);

	broken = FALSE;
	g_signal_connect (G_OBJECT (client), "broken",
			  G_CALLBACK (broken_cb), &broken);
	g_object_unref (G_OBJECT (server));
	g_assert (server == NULL);

	linc_main_iteration (FALSE); /* connect & break */

	g_assert (broken);

	g_object_unref (G_OBJECT (client));
	g_assert (client == NULL);
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

static void
blocking_cb (LINCConnection *cnx,
	     gulong          buffer_size,
	     gpointer        user_data)
{
	gulong *status = user_data;
	
	fprintf (stderr, " buffer %ld\n", buffer_size);

	(*status)++;

	if (buffer_size == BUFFER_MAX) {
		/*
		 * TODO: re-activate the server, flush the
		 * buffer, and test the 0 signal.
		 * Ensure that the data we read on the client
		 * side is in fact what we sent - in the right
		 * order.
		 */
	}
}

static void
test_blocking (void)
{
	LINCServer     *server;
	LINCConnection *client, *s_cnx;
	LINCWriteOpts  *options;
	GIOCondition    old_cond;
	guchar          buffer[1024] = { 0 };
	gulong          status = 0;
	int             i;

	fprintf (stderr, "Testing blocking code ...\n");

	create_server (&server);
	create_client (server, &client);
	linc_main_iteration (FALSE); /* connect */

	g_assert (server->priv->connections != NULL);
	s_cnx = server->priv->connections->data;
	g_assert (s_cnx != NULL);
	g_assert (s_cnx->priv->tag != NULL);
	old_cond = knobble_watch (s_cnx->priv->tag, 0); /* stop it listening */

	options = linc_write_options_new (FALSE);
	linc_connection_set_max_buffer (client, BUFFER_MAX);
	g_signal_connect (G_OBJECT (client), "blocking",
			  G_CALLBACK (blocking_cb), &status);
	client->options |= LINC_CONNECTION_BLOCK_SIGNAL;

	for (i = 0; i < SYS_SOCKET_BUFFER_MAX; i+= 128) {
		linc_connection_write (client, buffer, 128, options);
		if (client->status != LINC_CONNECTED)
			break;
	}

	g_assert (client->status == LINC_DISCONNECTED);
	g_assert (status == 2);

	g_object_unref (G_OBJECT (client));
	g_assert (client == NULL);

	knobble_watch (s_cnx->priv->tag, old_cond);
	linc_main_iteration (FALSE);

	g_object_unref (G_OBJECT (server));
	g_assert (server == NULL);
}

int
main (int argc, char **argv)
{	
	linc_init (FALSE);
	init_tmp ();

	test_broken ();
	test_blocking ();

	fprintf (stderr, "All tests passed successfully\n");
	
	return 0;
}
