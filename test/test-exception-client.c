
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include "test-exception.h"

/*   CONST */ 
#define PO_MAX_SEND 1

/*   VAR   */ 
static CORBA_ORB         global_orb;  // accessible from signal handler
static CORBA_Environment ev[1];

static gint server_pid=0;

static 
void
clear_sighandler (void)
{
  // ignore further signals
   struct sigaction ignore;

   ignore.sa_handler = SIG_IGN;

   sigemptyset (&ignore.sa_mask);

   ignore.sa_flags = 0;

   if (sigaction (SIGCHLD, &ignore, (struct sigaction *)0) == -1)
      abort ();

   if (sigaction (SIGINT, &ignore, (struct sigaction *)0) == -1)
      abort ();

   if (sigaction (SIGTERM, &ignore, (struct sigaction *)0) == -1)
      abort ();

   if (sigaction (SIGHUP, &ignore, (struct sigaction *)0) == -1)
      abort ();
}

static
void 
sighandler(int sig)
{
   int status=0;
   /*  Terminate event loop  */

   clear_sighandler();

   switch (sig)
   {
      case SIGCHLD:
	 /* check if child process (test-exception-server) did
	  * abort */
	 waitpid(server_pid, &status, 0);
	 g_assert (! WIFSIGNALED(status) && "server did abort");
	 break;
	
      default:
	 /* kill with brute force */
	 kill (server_pid, SIGKILL);
   }

   /*  Terminate event loop  */
   if (global_orb != CORBA_OBJECT_NIL)
   {
      CORBA_ORB_shutdown (global_orb, FALSE, NULL);
   }
}

static 
void
init_sighandler (void)
{
   struct sigaction sa;            /* New signal state */
   
   sa.sa_handler = sighandler; /* Set handler function */
   sigfillset(&sa.sa_mask);        /* Mask all other signals while handler
				      runs */
   sa.sa_flags =0 |SA_RESTART;     /* Restart interrupted syscalls */
   
   if (sigaction (SIGINT, &sa, (struct sigaction *)0) == -1)
      abort ();
   
   if (sigaction(SIGHUP, &sa, (struct sigaction *)0) == -1)
      abort ();
   
   if (sigaction(SIGTERM, &sa, (struct sigaction *)0) == -1)
      abort();

   if (sigaction(SIGCHLD, &sa, (struct sigaction *)0) == -1)
      abort();
}

static
int
client_main (int argc, char *argv[], FILE* istream)
{
   long i=0;
   gchar                           objref[1023+1];
   ORBitTestSuite_ExcptionFactory  obj = CORBA_OBJECT_NIL;

   CORBA_exception_init(ev);

   global_orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", ev);
   g_assert (ev->_major == CORBA_NO_EXCEPTION);

   if (-1==fscanf(istream,
		  "%1023s", objref)) /* limit chars to be read per line */
   {
      g_error ("failed on reading IOR from stdin");
      exit (1);
   }

   obj = (ORBitTestSuite_ExcptionFactory)
      CORBA_ORB_string_to_object (global_orb, 
				  objref, ev);   
   g_assert (ev->_major==CORBA_NO_EXCEPTION);
   g_assert (obj!=CORBA_OBJECT_NIL);
   
   /* trigger exceptions */
   for (i=0; i<100; ++i) 
   {
      ORBitTestSuite_ExcptionFactory_trigger_empty_exception(obj, 42, ev);
      g_assert (ev->_major == CORBA_USER_EXCEPTION);
      CORBA_exception_free (ev);
   }
   
   /* trigger exceptions */
   for (i=0; i<100; ++i) 
   {
      ORBitTestSuite_ExcptionFactory_trigger_empty_exception(obj, 42, ev);
      g_assert (ev->_major == CORBA_USER_EXCEPTION);
      /* FIXME, test for valid exception values */
      CORBA_exception_free (ev);
   }
   
   CORBA_Object_release(obj, ev);
   g_assert (ev->_major == CORBA_NO_EXCEPTION);

   CORBA_ORB_destroy(global_orb, ev);
   g_assert (ev->_major == CORBA_NO_EXCEPTION);

   return 0;
}

static
FILE*
start_server()
{
   FILE *istream=NULL;
   char *argv[]= {"test-exception-server", NULL};
   GError* err = NULL; 
   gint in=-1;
   gint ret=g_spawn_async_with_pipes (NULL,
				      argv, 
				      NULL,               /* env pointer */
				      0,                  /* spawn flags */
				      NULL,               /* spawn setup func*/
				      NULL,               /* user data */
				      &server_pid,        /* child pid */
				      NULL,               /* stdin */
				      &in,                /* stdout */
				      NULL,               /* stderr */ 
				      &err);              /* GError */
   g_assert (err==NULL);
   g_assert (ret==TRUE);
   g_assert (in!=-1);

   istream=fdopen(in, "r");
   return istream;
}

static
void
terminate_server (void)
{
   int status=0;
   int child_pid=0;

   /* ignore further SIGCHLD signals */ 
   clear_sighandler ();

   kill (server_pid, SIGINT);
   sleep (1);
   
   child_pid = waitpid (server_pid, &status, WNOHANG);
   if (child_pid==0)
   {
      /* child still alive, use brute force */
      kill (server_pid, SIGKILL);
      g_assert (!"server did not terminate on SIGINT");
   }
   
   /* check if child process (test-exception-server) did
    * abort */
   g_assert (! WIFSIGNALED(status) );
}

int
main (int argc, char* argv[])
{
   int ret=0;

   /*FIXME, option parsing with optarg */ 
   if (argc==1)
   {
      ret=client_main (argc, argv, stdin);
   }
   else if (argc==2 && strcmp ("--start-server", argv[1])==0)
   {
      FILE* istream=NULL;
      
      init_sighandler();
   
      istream=start_server ();  
      g_assert (istream!=NULL);
      
      ret=client_main (argc, argv, istream);
      
      terminate_server ();
      
      g_assert (fclose (istream) == 0);
   }
   else
   {
      g_print ("usage: %s [--start-server]\n", argv[0]);
      exit (1);
   }

   exit (ret);
}
