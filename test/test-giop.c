#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orbit/orbit.h>

GIOPServer     *server = NULL;
GIOPConnection *cnx = NULL;

int
main (int argc, char *argv[])
{
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	orb = CORBA_ORB_init (&argc, argv, "orbit-local-orb", &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);


	server = giop_server_new (GIOP_1_2, "UNIX", NULL, NULL, 0, orb);

	cnx = giop_connection_initiate (
		"UNIX",
		server->local_host_info,
		server->local_serv_info,
		LINC_CONNECTION_NONBLOCKING);
	g_assert (cnx != NULL);

	while (cnx->status != LINC_CONNECTED)
		linc_main_iteration (TRUE);

	

	CORBA_ORB_destroy (orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	CORBA_Object_release ((CORBA_Object) orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	return 0;
}
