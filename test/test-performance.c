#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <orbit/orbit.h>

static GTimer *timer;
static double  bogomark = 0.0;
static double  elapsed_time;

static void
test_copy (void)
{
	int i, j;
#define ELEMS (sizeof (tc) / sizeof (tc[0]))
	CORBA_TypeCode tc[] = {
		TC_CORBA_octet,
		TC_CORBA_sequence_CORBA_octet,
		TC_CORBA_double,
		TC_CORBA_string,
		TC_CORBA_sequence_CORBA_string,
		TC_GIOP_TargetAddress		
	};
	gpointer data [ELEMS];
	const char *test_string = "This is a sample string, for dupping";

	fprintf (stderr, "Testing copy...\n");

	for (i = 0; i < ELEMS; i++) {
		data [i] = ORBit_dynany_new_default (tc [i]);

		g_timer_reset (timer);
		for (j = 0; j < 1000; j++) {
			gpointer foo = ORBit_copy_value (data [i], tc [i]);
			CORBA_free (foo);
		}
		elapsed_time = g_timer_elapsed (timer, NULL);
		bogomark += elapsed_time;
		fprintf (stderr, " copy %20s : %g(ms)\n",
			tc[i]->repo_id == NULL ? "(null)" : tc[i]->repo_id,
			elapsed_time);
	}

	fprintf (stderr, "Testing strdup ...\n");
	
	g_timer_reset (timer);

	for (i = 0; i < 1000; i++) {
		char *str = CORBA_string_dup (test_string);
		CORBA_free (str);
	}
	elapsed_time = g_timer_elapsed (timer, NULL);
	bogomark += elapsed_time;
	fprintf (stderr, " strdup : %g(ms)\n", elapsed_time);
}

int
main (int argc, char *argv[])
{
	CORBA_Environment ev;
	CORBA_ORB    orb;

	free (malloc (8));

	CORBA_exception_init (&ev);

	timer = g_timer_new ();
	g_timer_start (timer);

	g_timer_reset (timer);
	orb = CORBA_ORB_init (NULL, NULL, "orbit-local-orb", &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	fprintf (stderr, "ORB: init took %g(ms)\n",
		 (elapsed_time = g_timer_elapsed (timer, NULL)) * 1000.0);
	bogomark += elapsed_time;

	test_copy ();

	g_timer_reset (timer);
	CORBA_ORB_destroy (orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);
	fprintf (stderr, "ORB: destroy took %g(ms)\n",
		 (elapsed_time = g_timer_elapsed (timer, NULL)) * 1000.0);
	bogomark += elapsed_time;
	g_timer_reset (timer);

	CORBA_Object_release ((CORBA_Object) orb, &ev);
	g_assert (ev._major == CORBA_NO_EXCEPTION);

	g_timer_destroy (timer);

	fprintf (stderr, "Overall bogomark %g\n", 1000.0 / bogomark);

	return 0;
}
