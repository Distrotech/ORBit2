#include <stdio.h>
#include <linc/linc.h>

gboolean broken;

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

int
main (int argc, char **argv)
{
	LINCServer     *server;
	LINCConnection *client;
	
	linc_init (FALSE);
	init_tmp ();

	server = g_object_new (linc_server_get_type (), NULL);

	g_assert (linc_server_setup (server, "UNIX", NULL, NULL,
				     LINC_CONNECTION_NONBLOCKING));

	client = g_object_new (linc_connection_get_type (), NULL);
	
	g_assert (linc_connection_initiate (
		client, "UNIX",
		server->local_host_info,
		server->local_serv_info,
		LINC_CONNECTION_NONBLOCKING));

	broken = FALSE;
	g_signal_connect (G_OBJECT (client), "broken",
			  G_CALLBACK (broken_cb), &broken);
	g_object_unref (G_OBJECT (server));

	g_assert (broken);

	g_object_unref (G_OBJECT (client));
	
	return 0;
}
