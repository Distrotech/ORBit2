#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "echo.h"
#include "echo-share.h"

/**
	This is used by echo-server.c and echo-local.c
	It uses echo-skels.c
**/

static Echo			the_echo_client;
static CORBA_ORB		the_orb;
static PortableServer_POA	the_poa;
static PortableServer_ObjectId*	the_objid;


static CORBA_Object
do_echoString(PortableServer_Servant servant,
	      const CORBA_char *astring,
	      CORBA_double *outnum,
	      CORBA_Environment *ev)
{
  if ( !echo_opt_quiet )
      g_message("[server] %s", astring);

  *outnum = rand() % 100;

#if 0
  {
  	PortableServer_POA myCurPoa;
  	PortableServer_ObjectId *myCurOid;
	CORBA_char *myCurOidStr;
	PortableServer_Current poacur = (PortableServer_Current)
	  CORBA_ORB_resolve_initial_references(the_orb, "POACurrent", ev);
  	g_assert( ev->_major == 0 );
  	myCurPoa = PortableServer_Current_get_POA(poacur,ev);
  	g_assert( ev->_major == 0 );
  	myCurOid = PortableServer_Current_get_object_id(poacur,ev);
  	g_assert( ev->_major == 0 );

	g_assert( myCurPoa == the_poa );
	g_assert( ORBit_sequence_octet_equal( myCurOid, the_objid) );
	myCurOidStr = PortableServer_ObjectId_to_string(myCurOid, ev);
  	if ( !echo_opt_quiet )
      	    g_message("[server] oid=%s", myCurOidStr);

  	CORBA_Object_release((CORBA_Object)myCurPoa,ev);
  	CORBA_free(myCurOid);
	CORBA_free(myCurOidStr);
  	CORBA_Object_release((CORBA_Object)poacur,ev);
  }
#endif

  return CORBA_Object_duplicate(the_echo_client, ev);
}

static void
do_doNothing(PortableServer_Servant servant, CORBA_Environment *ev) {
}

PortableServer_ServantBase__epv base_epv = {
  NULL,
  NULL,
  NULL
};
POA_Echo__epv echo_epv = { NULL, do_echoString, do_doNothing };
POA_Echo__vepv poa_echo_vepv = { &base_epv, &echo_epv };
POA_Echo poa_echo_servant = { NULL, &poa_echo_vepv };


void
echo_srv_start_poa(CORBA_ORB orb, CORBA_Environment *ev)
{
    PortableServer_POAManager mgr;

    the_orb = orb;
    the_poa = (PortableServer_POA)CORBA_ORB_resolve_initial_references(orb, 
      "RootPOA", ev);
    mgr = PortableServer_POA__get_the_POAManager(the_poa, ev);
    PortableServer_POAManager_activate(mgr, ev);
    CORBA_Object_release((CORBA_Object)mgr, ev);
}

CORBA_Object
echo_srv_start_object(CORBA_Environment *ev)
{
    POA_Echo__init(&poa_echo_servant, ev);
    if ( ev->_major ) {
	printf("object__init failed: %d\n", ev->_major);
	exit(1);
    }
    the_objid = PortableServer_POA_activate_object(the_poa,
		    &poa_echo_servant, ev);
    if ( ev->_major ) {
	printf("activate_object failed: %d\n", ev->_major);
	exit(1);
    }
    the_echo_client = PortableServer_POA_servant_to_reference(the_poa,
			    &poa_echo_servant, ev);
    if ( ev->_major ) {
	printf("servant_to_reference failed: %d\n", ev->_major);
	exit(1);
    }
    return the_echo_client;
}

void
echo_srv_finish_object(CORBA_Environment *ev)
{
    CORBA_Object_release(the_echo_client, ev);
    if ( ev->_major ) {
	printf("object_release failed: %d\n", ev->_major);
	exit(1);
    }
    the_echo_client = 0;
    PortableServer_POA_deactivate_object(the_poa, the_objid, ev);
    if ( ev->_major ) {
	printf("deactivate_object failed: %d\n", ev->_major);
	exit(1);
    }
    CORBA_free(the_objid);
    the_objid = 0;
    POA_Echo__fini(&poa_echo_servant, ev);
    if ( ev->_major ) {
	printf("object__fini failed: %d\n", ev->_major);
	exit(1);
    }
}


void
echo_srv_finish_poa(CORBA_Environment *ev)
{
    CORBA_Object_release((CORBA_Object)the_poa, ev);
    if ( ev->_major ) {
	printf("POA release failed: %d\n", ev->_major);
	exit(1);
    }
    the_poa = 0;
}
