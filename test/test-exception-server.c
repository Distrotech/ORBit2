#include <signal.h>
#include <stdlib.h>
#include <orbit/orbit.h>
#include <assert.h>
#include "test-exception.h"

/*   MACROS */
#define WARNING_IF_EXCEPTION(ev) if (ev->_major != CORBA_NO_EXCEPTION)      \
            { g_warning ("caught exception %s", CORBA_exception_id (ev));   \
              CORBA_exception_free (ev); }

#define ABORT_IF_EXCEPTION(ev) if (ev->_major != CORBA_NO_EXCEPTION)        \
            { g_error ("caught exception %s", CORBA_exception_id (ev));     \
              CORBA_exception_free (ev); abort(); }

#define IGNORE_IF_EXCEPTION(ev) if (ev->_major != CORBA_NO_EXCEPTION)       \
            {  CORBA_exception_free (ev); }


/*** App-specific servant structures ***/

typedef struct
{
   POA_ORBitTestSuite_ExcptionFactory servant;
   PortableServer_POA poa;

}
impl_POA_ORBitTestSuite_ExcptionFactory;

/*** Implementation stub prototypes ***/

static void
impl_ORBitTestSuite_ExcptionFactory__destroy
(impl_POA_ORBitTestSuite_ExcptionFactory * servant, CORBA_Environment * ev);
static void
impl_ORBitTestSuite_ExcptionFactory_trigger_empty_exception
(impl_POA_ORBitTestSuite_ExcptionFactory * servant, const CORBA_long pid,
CORBA_Environment * ev);

static void
impl_ORBitTestSuite_ExcptionFactory_trigger_value_exception
(impl_POA_ORBitTestSuite_ExcptionFactory * servant, const CORBA_long pid,
CORBA_Environment * ev);

/*** epv structures ***/

static PortableServer_ServantBase__epv
   impl_ORBitTestSuite_ExcptionFactory_base_epv = {
   NULL,			/* _private data */
   (gpointer) & impl_ORBitTestSuite_ExcptionFactory__destroy, /* finalize routine */
   NULL,			/* default_POA routine */
};
static POA_ORBitTestSuite_ExcptionFactory__epv
   impl_ORBitTestSuite_ExcptionFactory_epv = {
   NULL,			/* _private */
   (gpointer) & impl_ORBitTestSuite_ExcptionFactory_trigger_empty_exception,

   (gpointer) & impl_ORBitTestSuite_ExcptionFactory_trigger_value_exception,

};

/*** vepv structures ***/

static POA_ORBitTestSuite_ExcptionFactory__vepv
   impl_ORBitTestSuite_ExcptionFactory_vepv = {
   &impl_ORBitTestSuite_ExcptionFactory_base_epv,
   &impl_ORBitTestSuite_ExcptionFactory_epv,
};

/*** Stub implementations ***/

static ORBitTestSuite_ExcptionFactory
impl_ORBitTestSuite_ExcptionFactory__create(PortableServer_POA poa,
					    CORBA_Environment * ev)
{
   ORBitTestSuite_ExcptionFactory retval;
   impl_POA_ORBitTestSuite_ExcptionFactory *newservant;
   PortableServer_ObjectId *objid;

   newservant = g_new0(impl_POA_ORBitTestSuite_ExcptionFactory, 1);
   newservant->servant.vepv = &impl_ORBitTestSuite_ExcptionFactory_vepv;
   newservant->poa = poa;
   POA_ORBitTestSuite_ExcptionFactory__init((PortableServer_Servant)
					    newservant, ev);

   objid = PortableServer_POA_activate_object(poa, newservant, ev);
   CORBA_free(objid);
   retval = PortableServer_POA_servant_to_reference(poa, newservant, ev);

   return retval;
}

static void
impl_ORBitTestSuite_ExcptionFactory__destroy
   (impl_POA_ORBitTestSuite_ExcptionFactory * servant, CORBA_Environment * ev)
{
   PortableServer_ObjectId *objid;

   objid = PortableServer_POA_servant_to_id(servant->poa, servant, ev);
   PortableServer_POA_deactivate_object(servant->poa, objid, ev);
   CORBA_free(objid);

   POA_ORBitTestSuite_ExcptionFactory__fini((PortableServer_Servant) servant,
					    ev);
   g_free(servant);
}

static void
impl_ORBitTestSuite_ExcptionFactory_trigger_empty_exception
   (impl_POA_ORBitTestSuite_ExcptionFactory * servant, const CORBA_long pid,
    CORBA_Environment * ev)
{ 
    ORBitTestSuite_EmptyException* exdata
       = ORBitTestSuite_EmptyException__alloc();
    CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			 ex_ORBitTestSuite_EmptyException,
			 exdata);
}

static void
impl_ORBitTestSuite_ExcptionFactory_trigger_value_exception
   (impl_POA_ORBitTestSuite_ExcptionFactory * servant, const CORBA_long pid,
    CORBA_Environment * ev)
{
    ORBitTestSuite_ValueException* exdata
       = ORBitTestSuite_ValueException__alloc();
    exdata->minor_code = pid;
    exdata->mesg = CORBA_string_dup ("exception message");
   
    CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
			 ex_ORBitTestSuite_ValueException,
			 exdata);
}


/*   VAR   */
static CORBA_ORB  global_orb = CORBA_OBJECT_NIL; /*global declaration of orb*/
static CORBA_Environment ev[1];

static
void 
sighandler(int sig)
{
   // if using Linux-kernel 2.4 printing anything to console within
   // sighandler function is prohibited, as this might cause deadlocks

   /*  Terminate event loop  */

   if (global_orb != CORBA_OBJECT_NIL)
   {
      CORBA_ORB_shutdown (global_orb, FALSE, ev);
      IGNORE_IF_EXCEPTION(ev);
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
}

/*************************************************************************/
/****                            main                                 ****/
/*************************************************************************/

static 
int 
server_main (int argc, char* argv[])
{
   ORBitTestSuite_ExcptionFactory   servant;
   PortableServer_POA               root_poa;
   PortableServer_POAManager        pm;
   CORBA_char                      *objref = NULL;

   CORBA_exception_init(ev);

   /* create Object Request Broker (ORB) */
   global_orb = CORBA_ORB_init(&argc, argv, "orbit-local-orb", ev);
   ABORT_IF_EXCEPTION(ev);

   /* create Portable Object Adaptor (POA) */

   root_poa = (PortableServer_POA)
      CORBA_ORB_resolve_initial_references(global_orb,  "RootPOA", ev);
   ABORT_IF_EXCEPTION(ev);

   /* create servant instance(s) */   

   servant = impl_ORBitTestSuite_ExcptionFactory__create (root_poa, ev);
   ABORT_IF_EXCEPTION(ev);

   /* activate POA Manager */

   pm = PortableServer_POA__get_the_POAManager(root_poa, ev);
   ABORT_IF_EXCEPTION(ev);

   PortableServer_POAManager_activate(pm, ev);
   ABORT_IF_EXCEPTION(ev);

   /* write objref to file */

   objref = CORBA_ORB_object_to_string(global_orb, servant, ev);
   ABORT_IF_EXCEPTION(ev);

   /* print ior to terminal */
   g_print ("%s\n", objref);

   CORBA_free (objref); 
   objref = NULL;

   /* enter main loop */
 
   CORBA_ORB_run(global_orb, ev);
   ABORT_IF_EXCEPTION(ev);

   /* main loop has been left due to CORBA_ORB_shutdown(.) */

   if (global_orb != CORBA_OBJECT_NIL)
   {
      /* going to destroy orb.. */
      CORBA_ORB_destroy(global_orb, ev);
      ABORT_IF_EXCEPTION(ev);
   }
   return 0;
}

int
main(int argc, char *argv[])
{
   int ret=0;
   init_sighandler ();
   ret=server_main(argc, argv);
   exit (ret);
}


