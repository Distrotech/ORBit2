/*
 * CORBA C language mapping tests
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Phil Dawes <philipd@users.sourceforge.net>
 */

#undef DO_HARDER_SEGV

#include "everything.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Singleton accessor for the test factory */
test_TestFactory getFactoryInstance(CORBA_Environment *ev);

typedef void (*init_fn_t) (PortableServer_Servant, CORBA_Environment *);

CORBA_Object create_object (PortableServer_POA poa,
			    init_fn_t          fn,
			    gpointer           servant,
			    CORBA_Environment *ev);

CORBA_ORB          global_orb;
PortableServer_POA global_poa;

#include "basicServer.c"
#include "structServer.c"
#include "sequenceServer.c"
#include "unionServer.c"
#include "arrayServer.c"
#include "anyServer.c"
#include "contextServer.c"
#include "deadReference.c"
#include "pingServer.c"

typedef struct {
	POA_test_TestFactory baseServant;

	test_BasicServer     basicServerRef;
	test_StructServer    structServerRef;
	test_SequenceServer  sequenceServerRef;
	test_UnionServer     unionServerRef;
	test_ArrayServer     arrayServerRef;
	test_AnyServer       anyServerRef;
	test_ContextServer   contextServerRef;
	GSList              *pingPongServerRefs;
} test_TestFactory_Servant;

static test_BasicServer
TestFactory_getBasicServer (PortableServer_Servant servant,
			    CORBA_Environment     *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant *) servant;

	return CORBA_Object_duplicate (this->basicServerRef, ev);
}

static CORBA_char *
TestFactory_getStructServerIOR (PortableServer_Servant servant,
				CORBA_Environment     *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant *) servant;

	return CORBA_ORB_object_to_string (
		ORBIT_SERVANT_TO_FIRST_POAOBJECT (servant)->poa->orb,
		this->structServerRef, ev);
}

static test_StructServer
TestFactory_getStructServer (PortableServer_Servant servant,
			     CORBA_Environment     *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant*) servant;

	return CORBA_Object_duplicate (this->structServerRef, ev);
}

static
test_SequenceServer
TestFactory_getSequenceServer(PortableServer_Servant servant,
			      CORBA_Environment     *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant*) servant;

	return CORBA_Object_duplicate(this->sequenceServerRef,ev);
}

static test_UnionServer
TestFactory_getUnionServer (PortableServer_Servant servant,
			    CORBA_Environment     *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant*) servant;

	return CORBA_Object_duplicate (this->unionServerRef, ev);
}

static test_ArrayServer
TestFactory_getArrayServer (PortableServer_Servant servant,
			    CORBA_Environment     *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant*) servant;

	return CORBA_Object_duplicate (this->arrayServerRef, ev);
}

static test_AnyServer
TestFactory_getAnyServer (PortableServer_Servant servant,
			  CORBA_Environment     *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant*) servant;

	return CORBA_Object_duplicate (this->anyServerRef, ev);
}

static test_ContextServer
TestFactory_getContextServer (PortableServer_Servant servant,
			      CORBA_Environment     *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant*) servant;

	return CORBA_Object_duplicate (this->contextServerRef, ev);
}

static test_PingPongServer
TestFactory_createPingPongServer (PortableServer_Servant servant,
				  CORBA_Environment     *ev)
{
	test_PingPongServer obj;

	obj = create_object (
		global_poa, POA_test_PingPongServer__init,
		create_ping_pong_servant (), ev);

	if (servant) {
		test_TestFactory_Servant *this;

		this = (test_TestFactory_Servant*) servant;

		this->pingPongServerRefs = g_slist_prepend (
			this->pingPongServerRefs, obj);
	}

	return CORBA_Object_duplicate (obj, ev);
}

static test_DeadReferenceObj
TestFactory_createDeadReferenceObj (PortableServer_Servant  servant,
				    CORBA_Environment      *ev)
{
	PortableServer_Current    poa_current;
        PortableServer_POA        poa;
	CORBA_Object              obj;

        poa_current = (PortableServer_Current)
			CORBA_ORB_resolve_initial_references (global_orb,
							      "POACurrent",
							      ev);

	g_assert (ev->_major == CORBA_NO_EXCEPTION);

        poa = PortableServer_Current_get_POA (poa_current, ev);

	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	obj = create_object (poa, POA_test_DeadReferenceObj__init,
			     &DeadReferenceObj_servant, ev);

	CORBA_Object_release ((CORBA_Object) poa, ev);
	CORBA_Object_release ((CORBA_Object) poa_current, ev);

	/* Note: Not duping - ORB will free it and reference
	 * should dangle. */
	return obj;
}

static CORBA_char *
TestFactory_segv (PortableServer_Servant servant,
		  const CORBA_char      *when,
		  CORBA_Environment     *ev)
{
#ifdef DO_HARDER_SEGV
	/* Emulate a SegV */
	exit (0);
#else
	g_main_loop_quit (linc_loop);
#endif

	return CORBA_string_dup ("I'm dead");
}

static void
test_TestFactory__fini (PortableServer_Servant  servant,
			CORBA_Environment      *ev)
{
	GSList                   *l;
	test_TestFactory_Servant *this;

	this = (test_TestFactory_Servant*) servant;

	CORBA_Object_release (this->basicServerRef, ev);
	CORBA_Object_release (this->structServerRef, ev);
	CORBA_Object_release (this->sequenceServerRef, ev);
	CORBA_Object_release (this->unionServerRef, ev);
	CORBA_Object_release (this->arrayServerRef, ev);
	CORBA_Object_release (this->anyServerRef, ev);
	CORBA_Object_release (this->contextServerRef, ev);

	for (l = this->pingPongServerRefs; l; l = l->next)
		CORBA_Object_release (l->data, ev);
	g_slist_free (this->pingPongServerRefs);
}


/* vtable */
static PortableServer_ServantBase__epv TestFactory_base_epv = {
	NULL,
	test_TestFactory__fini,
	NULL
};

static POA_test_TestFactory__epv TestFactory_epv = {
	NULL,
	TestFactory_getBasicServer,
	TestFactory_getStructServer,
	TestFactory_getStructServerIOR,
	TestFactory_getSequenceServer,
	TestFactory_getUnionServer,
	TestFactory_getArrayServer,
	TestFactory_getAnyServer,
	TestFactory_getContextServer,
	TestFactory_segv,
	NULL,                         /* getBaseServer                */
	NULL,                         /* getDerivedServer             */
	NULL,                         /* getDerivedServerAsBaseServer */
	NULL,                         /* getDerivedServerAsB2         */
	NULL,                         /* createTransientObj           */
	TestFactory_createDeadReferenceObj,
	TestFactory_createPingPongServer
};

static POA_test_TestFactory__vepv TestFactory_vepv = {
	&TestFactory_base_epv,
	&TestFactory_epv
};

static PortableServer_POA
start_poa (CORBA_ORB orb, CORBA_Environment *ev)
{
	PortableServer_POAManager mgr;
	PortableServer_POA the_poa;

	the_poa = (PortableServer_POA) CORBA_ORB_resolve_initial_references (
		orb, "RootPOA", ev);

	mgr = PortableServer_POA__get_the_POAManager (the_poa, ev);
	PortableServer_POAManager_activate (mgr, ev);
	CORBA_Object_release ((CORBA_Object) mgr, ev);

	return the_poa;
}

CORBA_Object
create_object (PortableServer_POA poa,
	       init_fn_t          fn,
	       gpointer           servant,
	       CORBA_Environment *ev)
{
	CORBA_Object             object;
	PortableServer_ObjectId *objid;

	fn (servant, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		g_error ("object__init failed: %d\n", ev->_major);
	g_assert (ORBIT_SERVANT_TO_CLASSINFO (servant) != NULL);

	objid = PortableServer_POA_activate_object (
		poa, servant, ev);

	if (ev->_major != CORBA_NO_EXCEPTION)
		g_error ("activate_object failed: %d\n", ev->_major);

	object = PortableServer_POA_servant_to_reference (
		poa, servant, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		g_error ("servant_to_reference failed: %d\n", ev->_major);

	g_assert (ORBIT_STUB_GetServant (object) == servant);
	g_assert (ORBIT_SERVANT_TO_CLASSINFO (servant) != NULL);

	CORBA_free (objid);

	return object;
}

/* constructor */
static void
test_TestFactory__init (PortableServer_Servant servant, 
			PortableServer_POA poa, 
			CORBA_Environment *ev)
{
	test_TestFactory_Servant *this = (test_TestFactory_Servant*)servant;  
	this->baseServant._private = NULL;
	this->baseServant.vepv = &TestFactory_vepv;

	this->basicServerRef = create_object (
		poa, POA_test_BasicServer__init,
		&BasicServer_servant, ev);

	this->structServerRef = create_object (
		poa, POA_test_StructServer__init,
		&StructServer_servant, ev);

	this->sequenceServerRef = create_object (
		poa, POA_test_SequenceServer__init,
		&SequenceServer_servant, ev);

	this->unionServerRef = create_object (
		poa, POA_test_UnionServer__init,
		&UnionServer_servant, ev);

	this->arrayServerRef = create_object (
		poa, POA_test_ArrayServer__init,
		&ArrayServer_servant, ev);

	this->anyServerRef = create_object (
		poa, POA_test_AnyServer__init,
		&AnyServer_servant, ev);

	this->contextServerRef = create_object (
		poa, POA_test_ContextServer__init,
		&ContextServer_servant, ev);

	this->pingPongServerRefs = NULL;

	POA_test_TestFactory__init (
		(PortableServer_ServantBase *) servant, ev);
}

static test_TestFactory factory;

test_TestFactory
getFactoryInstance (CORBA_Environment *ev)
{
	return CORBA_Object_duplicate (factory, ev);
}

#ifndef _IN_CLIENT_
static int
dump_ior (CORBA_ORB orb, const char *fname, CORBA_Environment *ev)
{
	FILE *outfile;
	CORBA_char *ior;
	
	outfile = fopen ("iorfile","wb");

	g_return_val_if_fail (outfile != NULL, 1);
	g_return_val_if_fail (factory != CORBA_OBJECT_NIL, 1);

	ior = CORBA_ORB_object_to_string (orb, factory, ev);
	g_return_val_if_fail (ior != NULL, 1);

	fwrite (ior, strlen (ior), 1, outfile);

	fclose (outfile);

	CORBA_free (ior);

	return 0;
}
#endif

static CORBA_Object
create_TestFactory (PortableServer_POA        poa,
		    test_TestFactory_Servant *servant,
		    CORBA_Environment        *ev)
{
	CORBA_Object             object;
	PortableServer_ObjectId *objid;

	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	test_TestFactory__init (servant, poa, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		g_error ("object__init failed: %d\n", ev->_major);
	g_assert (ORBIT_SERVANT_TO_CLASSINFO (servant) != NULL);

	objid = PortableServer_POA_activate_object (
		poa, servant, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		g_error ("activate_object failed: %d\n", ev->_major);

	object = PortableServer_POA_servant_to_reference (
		poa, servant, ev);
	if (ev->_major != CORBA_NO_EXCEPTION)
		g_error ("servant_to_reference failed: %d\n", ev->_major);

	g_assert (ORBIT_STUB_GetServant (object) == servant);
	g_assert (ORBIT_SERVANT_TO_CLASSINFO (servant) != NULL);

	CORBA_free (objid);

	return object;
}

test_TestFactory_Servant servant;

#ifndef _IN_CLIENT_
  int
  main (int argc, char *argv [])
#else
  static CORBA_Object
  get_server (CORBA_ORB orb,
	      CORBA_Environment *ev)
#endif
{
	test_BasicServer objref;
#ifndef _IN_CLIENT_
	CORBA_Environment real_ev;
	CORBA_Environment *ev = &real_ev;

/*	g_mem_set_vtable (glib_mem_profiler_table); */

	free (malloc (8)); /* -lefence */

	CORBA_exception_init(&real_ev);

	/* Initialize threads first - so we get a threaded ORB */
	if (!g_thread_supported ())
		g_thread_init (NULL);

	global_orb = CORBA_ORB_init (&argc, argv, "", ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
#endif

	global_poa = start_poa (global_orb, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	factory = create_TestFactory (global_poa, &servant, ev);
	g_assert (factory != CORBA_OBJECT_NIL);

	/* a quick local test */
	objref = test_TestFactory_getBasicServer (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	g_assert (objref != CORBA_OBJECT_NIL);
	g_assert (CORBA_Object_is_a (objref, "IDL:orbit/test/BasicServer:1.0", ev));
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	CORBA_Object_release (objref, ev);
	g_assert(ev->_major == CORBA_NO_EXCEPTION);
	fprintf (stderr, "Local server test passed\n");

#ifndef _IN_CLIENT_
	if (!dump_ior (global_orb, "iorfile", ev))
		CORBA_ORB_run (global_orb, ev);

	CORBA_Object_release ((CORBA_Object) global_poa, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_Object_release (factory, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_ORB_destroy (global_orb, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);

	CORBA_Object_release ((CORBA_Object) global_orb, ev);
	g_assert (ev->_major == CORBA_NO_EXCEPTION);
	
	CORBA_exception_free (ev);

	return 0;
#else
	return factory;
#endif
}
