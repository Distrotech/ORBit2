#include <config.h>
#include <orbit/orbit.h>
#include "orbit-poa.h"
#include "../util/orbit-purify.h"

static void
ORBit_POAManager_free_fn (ORBit_RootObject obj)
{
	PortableServer_POAManager poa_mgr = (PortableServer_POAManager)obj;

	g_assert (poa_mgr->poa_collection == NULL);
	p_free (poa_mgr, struct PortableServer_POAManager_type);
}

static const ORBit_RootObject_Interface CORBA_POAManager_epv = {
	ORBIT_ROT_POAMANAGER,
	ORBit_POAManager_free_fn
};

PortableServer_POAManager
ORBit_POAManager_new (CORBA_ORB orb)
{
	PortableServer_POAManager retval;

	retval = g_new0 (struct PortableServer_POAManager_type, 1);
	ORBit_RootObject_init (&retval->parent, &CORBA_POAManager_epv);
	retval->state = PortableServer_POAManager_HOLDING;
	retval->orb = orb;

	return retval;
}

void
ORBit_POAManager_register_poa (PortableServer_POAManager  poa_mgr, 
			       PortableServer_POA         poa)
{
	g_assert (g_slist_find (poa_mgr->poa_collection, poa) == NULL);

	poa_mgr->poa_collection = g_slist_append (poa_mgr->poa_collection, poa);
}

void
ORBit_POAManager_unregister_poa (PortableServer_POAManager poa_mgr, 
				 PortableServer_POA        poa)
{
	poa_mgr->poa_collection = g_slist_remove (poa_mgr->poa_collection, poa);
}

/*
 * PortableServer::POAManager interface.
 * Section 11.3.2
 */

void
PortableServer_POAManager_activate (PortableServer_POAManager  manager,
				    CORBA_Environment         *ev)
{
	GSList *l;

	if (!manager) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM,
					    CORBA_COMPLETED_NO);
		return;
	}

	if (manager->state == PortableServer_POAManager_INACTIVE) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POAManager_AdapterInactive,
				     NULL);
		return;
	}

	manager->state = PortableServer_POAManager_ACTIVE;

	for (l = manager->poa_collection; l; l = l->next) {
		PortableServer_POA poa = (PortableServer_POA)l->data;

		ORBit_POA_handle_held_requests (poa);
	}
}

void
PortableServer_POAManager_hold_requests (PortableServer_POAManager  manager,
					 const CORBA_boolean        wait_for_completion,
					 CORBA_Environment         *ev)
{
	if (!manager) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM,
					    CORBA_COMPLETED_NO);
		return;
	}

	if (manager->state == PortableServer_POAManager_INACTIVE) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POAManager_AdapterInactive,
				     NULL);
		return;
	}

	manager->state = PortableServer_POAManager_HOLDING;

	if (!wait_for_completion)
		g_warning ("hold_requests not finished - don't know how to kill "
			   "outstanding request fulfillments");
}

void
PortableServer_POAManager_discard_requests (PortableServer_POAManager  manager,
					    const CORBA_boolean        wait_for_completion,
					    CORBA_Environment         *ev)
{
	if (!manager) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM,
					    CORBA_COMPLETED_NO);
		return;
	}

	if (manager->state == PortableServer_POAManager_INACTIVE) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POAManager_AdapterInactive,
				     NULL);
		return;
	}

	manager->state = PortableServer_POAManager_DISCARDING;

	if (!wait_for_completion)
		g_warning ("discard_requests not finished - don't know how to kill "
			   "outstanding request fulfillments");
}

void
PortableServer_POAManager_deactivate (PortableServer_POAManager  manager,
				      const CORBA_boolean        etherealize_objects,
				      const CORBA_boolean        wait_for_completion,
				      CORBA_Environment         *ev)
{
	GSList *l;

	if (!manager) {
		CORBA_exception_set_system (ev, ex_CORBA_BAD_PARAM,
					    CORBA_COMPLETED_NO);
		return;
	}

	if (manager->state == PortableServer_POAManager_INACTIVE) {
		CORBA_exception_set (ev, CORBA_USER_EXCEPTION,
				     ex_PortableServer_POAManager_AdapterInactive,
				     NULL);
		return;
	}

	if (wait_for_completion)
		for(l = manager->poa_collection; l; l = l->next)
			if (!ORBit_POA_is_inuse (l->data, FALSE, ev)) {
				CORBA_exception_set_system (ev,
							    ex_CORBA_BAD_INV_ORDER,
							    CORBA_COMPLETED_NO);
				return;
			}

	manager->state = PortableServer_POAManager_INACTIVE;

	for (l = manager->poa_collection; l; l = l->next)
		ORBit_POA_deactivate (l->data, etherealize_objects, ev);
}

PortableServer_POAManager_State
PortableServer_POAManager_get_state (PortableServer_POAManager  manager,
				     CORBA_Environment         *ev)
{
	return manager->state;
}
