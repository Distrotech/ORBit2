#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orbit/orbit.h>

#include "test-giop-frag.h"

LINCWriteOpts  *non_blocking = NULL;
GIOPServer     *server = NULL;
GIOPConnection *cnx = NULL;

static void
test_fragments (void)
{
	int i;
	GIOPRecvBuffer *buf;
	GIOPMessageQueueEntry ent;

	linc_connection_write (
		LINC_CONNECTION (cnx),
		giop_fragment_data,
		sizeof (giop_fragment_data),
		non_blocking);

	memset (&ent, 0, sizeof (ent));
	ent.cnx = cnx;
	ent.request_id = giop_fragment_request_id;

	for (i = 0; i < 1000; i++)
		linc_main_iteration (FALSE);
}

int
main (int argc, char *argv[])
{
	CORBA_ORB orb;
	CORBA_Environment ev;

	CORBA_exception_init (&ev);

	orb = CORBA_ORB_init (&argc, argv, "orbit-local-orb", &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	non_blocking = linc_write_options_new (FALSE);

	server = giop_server_new (GIOP_1_2, "UNIX", NULL, NULL, 0, orb);
	g_assert (LINC_IS_SERVER (server));

	cnx = giop_connection_initiate (
		"UNIX",
		LINC_SERVER (server)->local_host_info,
		LINC_SERVER (server)->local_serv_info,
		LINC_CONNECTION_NONBLOCKING,
		GIOP_1_2);
	g_assert (cnx != NULL);

	while (LINC_CONNECTION (cnx)->status != LINC_CONNECTED)
		linc_main_iteration (TRUE);


	test_fragments ();


	linc_write_options_free (non_blocking);
	CORBA_ORB_destroy (orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	CORBA_Object_release ((CORBA_Object) orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	return 0;
}
