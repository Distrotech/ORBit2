#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include "echo.h"
#include "echo-share.h"

/*public*/ gboolean echo_opt_quiet = 0;

int
main (int argc, char *argv[])
{
    CORBA_Environment ev;
    CORBA_ORB orb;
    Echo echo_client = CORBA_OBJECT_NIL;
    char *retval;

    signal(SIGINT, exit);
    signal(SIGTERM, exit);

    CORBA_exception_init(&ev);
    orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", &ev);
    g_assert(ev._major == CORBA_NO_EXCEPTION);

    echo_srv_start_poa(orb, &ev);
    g_assert(ev._major == CORBA_NO_EXCEPTION);
    echo_client = echo_srv_start_object(&ev);
    retval = CORBA_ORB_object_to_string(orb, echo_client, &ev);
    g_assert(ev._major == CORBA_NO_EXCEPTION);
    fprintf(stdout, "%s\n", retval); fflush(stdout);
    CORBA_free(retval);

    CORBA_ORB_run(orb, &ev);

    echo_srv_finish_object(&ev);
    echo_srv_finish_poa(&ev);
    CORBA_exception_free(&ev);

    return 0;
}
