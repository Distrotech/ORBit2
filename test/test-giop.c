#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orbit/orbit.h>
#include "GIOP/giop-debug.h"

#include "test-giop-frag.h"

LINCWriteOpts  *non_blocking = NULL;
GIOPServer     *server = NULL;
GIOPConnection *cnx = NULL;

gboolean global_flag;

static void
hook_unexpected_frag_reply (GIOPRecvBuffer *buf)
{
	char *p;
	const char testa[] = "ADVENTURE";  /* cf. Willard Price */
	const char testb[] = "MILLENNIUM"; /* cf. Robbie Williams */
	const char testc[] = "It isn't,  said the Caterpillar";
	const char testd[] = "Why?  said the Caterpillar";

	global_flag = TRUE;

	g_assert (buf != NULL);
	g_assert (buf->left_to_read == 0);
	g_assert (buf->msg.header.message_size == 1727);

	p = buf->message_body + 52;
	g_assert (!strncmp (p, testa, sizeof (testa) - 1));

	p = buf->message_body + 97;
	g_assert (!strncmp (p, testb, sizeof (testb) - 1));

	p = buf->message_body + 1002;
	g_assert (!strncmp (p, testc, sizeof (testc) - 1));

	p = buf->message_body + 1702;
	g_assert (!strncmp (p, testd, sizeof (testd) - 1));
}

static void
test_fragments (void)
{
	fprintf (stderr, "Testing fragment support ...\n");

	linc_connection_write (
		LINC_CONNECTION (cnx),
		giop_fragment_data,
		sizeof (giop_fragment_data),
		non_blocking);

	giop_debug_hook_unexpected_reply = hook_unexpected_frag_reply;

	global_flag = FALSE;
	while (!global_flag)
		linc_main_iteration (FALSE);

	giop_debug_hook_unexpected_reply = NULL;
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

	g_object_unref (server);
	g_object_unref (cnx);

	linc_write_options_free (non_blocking);
	CORBA_ORB_destroy (orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	CORBA_Object_release ((CORBA_Object) orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	fprintf (stderr, "All tests passed.\n");

	return 0;
}
