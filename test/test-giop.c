#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orbit/orbit.h>
#include "GIOP/giop-debug.h"

#include "test-giop-frag.h"

#ifndef G_ENABLE_DEBUG
#  warning GIOP test hooks only enabled in a debugging build
int
main (int argc, char *argv[])
{
	g_warning ("GIOP test hooks only enabled in a debugging build");
	return 0;
}
#else

LINCWriteOpts  *non_blocking = NULL;
GIOPServer     *server     = NULL;
GIOPConnection *server_cnx = NULL;
GIOPConnection *cnx        = NULL;

gboolean fragment_done;

static void
wait_for_disconnect (void)
{
	int i;

	/* a main_pending just looks for IO and not HUPs */
	for (i = 0; i < 10; i++) {
		if (linc_main_pending ())
			i = 0;
		linc_main_iteration (FALSE);
	}
}

static void
hook_unexpected_frag_reply (GIOPRecvBuffer *buf)
{
	char *p;
	const char testa[] = "ADVENTURE";  /* cf. Willard Price */
	const char testb[] = "MILLENNIUM"; /* cf. Robbie Williams */
	const char testc[] = "It isn't,  said the Caterpillar";
	const char testd[] = "Why?  said the Caterpillar";

	fragment_done = TRUE;

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

	fragment_done = FALSE;
	while (!fragment_done)
		linc_main_iteration (FALSE);

	giop_debug_hook_unexpected_reply = NULL;
}

gboolean spoof_done;
gboolean spoof_succeeded;

static void
test_spoof_callback (GIOPMessageQueueEntry *ent)
{
	spoof_done = spoof_succeeded = TRUE;
}

static void
test_spoof_hook (GIOPRecvBuffer *buffer,
		 GIOPMessageQueueEntry *ent)
{
	spoof_done = TRUE;
}

static void
test_spoofing (void)
{
	int i;
	GIOPConnection *misc;
	GIOPSendBuffer *reply;
	GIOPMessageQueueEntry ent;
	CORBA_unsigned_long request_id;

	fprintf (stderr, "Testing spoofing\n");

	request_id = 0x12345;
	giop_debug_hook_spoofed_reply = test_spoof_hook;
	misc = g_object_new (giop_connection_get_type (), NULL);

	for (i = 0; i < 2; i++) {
		giop_recv_list_setup_queue_entry (&ent, !i ? server_cnx : misc,
						  GIOP_REPLY, request_id);
		giop_recv_list_setup_queue_entry_async (&ent, test_spoof_callback);

		reply = giop_send_buffer_use_reply (
			GIOP_1_2, request_id , CORBA_NO_EXCEPTION);

		spoof_done = FALSE;
		spoof_succeeded = FALSE;
		
		g_assert (!giop_send_buffer_write (reply, cnx, TRUE));
	
		giop_send_buffer_unuse (reply);

		while (!spoof_done)
			linc_main_iteration (TRUE);

		switch (i) {
		case 0: /* valid */
			g_assert (spoof_succeeded);
			break;
		case 1: /* invalid */
			g_assert (!spoof_succeeded);
			wait_for_disconnect ();
			g_assert (LINC_CONNECTION (cnx)->status == LINC_DISCONNECTED);
			break;
		default:
			g_assert_not_reached ();
			break;
		}
	}
	g_object_unref (misc);
	giop_debug_hook_spoofed_reply = NULL;
}

static void
run_test_hook_new_connection (GIOPServer     *server,
			      GIOPConnection *new_cnx)
{
	g_assert (g_type_is_a (G_TYPE_FROM_INSTANCE (server),
			       GIOP_TYPE_SERVER));
	g_assert (g_type_is_a (G_TYPE_FROM_INSTANCE (new_cnx),
			       GIOP_TYPE_CONNECTION));

	server_cnx = new_cnx;
}

static void
run_test (CORBA_ORB orb, void (*do_test) (void), gboolean reverse)
{
	server = giop_server_new (GIOP_1_2, "UNIX", NULL, NULL, 0, orb);
	server_cnx = NULL;
	g_assert (LINC_IS_SERVER (server));

	giop_debug_hook_new_connection = run_test_hook_new_connection;

	cnx = giop_connection_initiate (
		"UNIX",
		LINC_SERVER (server)->local_host_info,
		LINC_SERVER (server)->local_serv_info,
		LINC_CONNECTION_NONBLOCKING,
		GIOP_1_2);
	g_assert (cnx != NULL);

	while (server_cnx == NULL)
		linc_main_iteration (TRUE);

	giop_debug_hook_new_connection = NULL;
	g_assert (server_cnx != NULL);

	if (reverse) {
		gpointer tmp = server_cnx;
		server_cnx = cnx;
		cnx = tmp;
	}


	do_test ();

	if (reverse) {
		gpointer tmp = server_cnx;
		server_cnx = cnx;
		cnx = tmp;
	}

	g_object_unref (server);
	server_cnx = NULL;
	server = NULL;
	g_object_unref (cnx);
	cnx = NULL;
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

	run_test (orb, test_fragments, FALSE);
	run_test (orb, test_fragments, TRUE);
	run_test (orb, test_spoofing, FALSE);
	run_test (orb, test_spoofing, TRUE);

	linc_write_options_free (non_blocking);
	CORBA_ORB_destroy (orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	CORBA_Object_release ((CORBA_Object) orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	fprintf (stderr, "All tests passed.\n");

	return 0;
}

#endif /* G_ENABLE_DEBUG */
