#include <orbit/orbit.h>
#include "orbit-poa.h"
#include <string.h>

/************** POAManager */
static void
ORBit_POAManager_free_fn(ORBit_RootObject obj_in)
{
  PortableServer_POAManager poa_mgr = (PortableServer_POAManager)obj_in;
  g_assert(poa_mgr->poa_collection == NULL);
  g_free(poa_mgr);
}

static const ORBit_RootObject_Interface CORBA_POAManager_epv = {
	ORBIT_ROT_POAMANAGER,
	ORBit_POAManager_free_fn
};

PortableServer_POAManager
ORBit_POAManager_new(CORBA_ORB orb, CORBA_Environment *ev)
{
  PortableServer_POAManager retval;

  retval = g_new0(struct PortableServer_POAManager_type, 1);
  ORBit_RootObject_init(&retval->parent, &CORBA_POAManager_epv);
  retval->state = PortableServer_POAManager_HOLDING;
  retval->orb = orb;

  return retval;
}

void
ORBit_POAManager_register_poa(PortableServer_POAManager poa_mgr, 
			      PortableServer_POA poa,
			      CORBA_Environment *ev)
{
  g_assert( g_slist_find(poa_mgr->poa_collection, poa) == 0 );
  poa_mgr->poa_collection = 
    g_slist_append(poa_mgr->poa_collection, poa);
  g_assert(poa->poa_manager == 0 );
  poa->poa_manager = ORBit_RootObject_duplicate(poa_mgr);
}

void
ORBit_POAManager_unregister_poa(PortableServer_POAManager poa_mgr, 
				PortableServer_POA poa,
				CORBA_Environment *ev)
{
  poa_mgr->poa_collection = g_slist_remove(poa_mgr->poa_collection, poa);
  g_assert(poa->poa_manager == poa_mgr);
  poa->poa_manager = NULL;
  ORBit_RootObject_release(poa_mgr);
}

/**** PortableServer_POAManager_activate
      Inputs: 'obj' - a POAManager to activate
      Outputs: '*ev' - result of the activate operation

      Side effect: Clears the 'held_requests' lists for all POA's
                   associated with the 'obj' POAManager.

      Description: Sets the POAManager state to 'ACTIVE', then
                   goes through all the POA's associated with this
		   POAManager, and makes them re-process their
		   'held_requests'
 */
void
PortableServer_POAManager_activate(PortableServer_POAManager obj,
				   CORBA_Environment *ev)
{
  GSList *curitem;
  PortableServer_POA curpoa;

  if(!obj)
    {
      CORBA_exception_set_system(ev,
				 ex_CORBA_BAD_PARAM,
				 CORBA_COMPLETED_NO);
      return;
    }

  if(obj->state == PortableServer_POAManager_INACTIVE)
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_POAManager_AdapterInactive,
			  NULL);
      return;
    }

  obj->state = PortableServer_POAManager_ACTIVE;

  for(curitem = obj->poa_collection; curitem;
      curitem = g_slist_next(curitem))
    {
      curpoa = (PortableServer_POA)curitem->data;
      ORBit_POA_handle_held_requests(curpoa);
    }

  ev->_major = CORBA_NO_EXCEPTION;
}

void
PortableServer_POAManager_hold_requests(PortableServer_POAManager _obj,
					const CORBA_boolean wait_for_completion,
					CORBA_Environment * ev)
{
  if(!_obj)
    {
      CORBA_exception_set_system(ev,
				 ex_CORBA_BAD_PARAM,
				 CORBA_COMPLETED_NO);
      return;
    }

  if(_obj->state == PortableServer_POAManager_INACTIVE)
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_POAManager_AdapterInactive,
			  NULL);
      return;
    }

  _obj->state = PortableServer_POAManager_HOLDING;
  if(!wait_for_completion)
    g_warning("hold_requests not finished - don't know how to kill outstanding request fulfillments");

  ev->_major = CORBA_NO_EXCEPTION;
}

void
PortableServer_POAManager_discard_requests(PortableServer_POAManager _obj,
					   const CORBA_boolean wait_for_completion,
					   CORBA_Environment * ev)
{
  if(!_obj)
    {
      CORBA_exception_set_system(ev,
				 ex_CORBA_BAD_PARAM,
				 CORBA_COMPLETED_NO);
      return;
    }

  if(_obj->state == PortableServer_POAManager_INACTIVE)
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_POAManager_AdapterInactive,
			  NULL);
      return;
    }

  _obj->state = PortableServer_POAManager_DISCARDING;
  if(!wait_for_completion)
    g_warning("discard_requests not finished - don't know how to kill outstanding request fulfillments");
  ev->_major = CORBA_NO_EXCEPTION;
}

void
PortableServer_POAManager_deactivate(PortableServer_POAManager _obj,
				     const CORBA_boolean etherealize_objects,
				     const CORBA_boolean wait_for_completion,
				     CORBA_Environment * ev)
{
  GSList	*poai;
  ev->_major = CORBA_NO_EXCEPTION;
  if(!_obj)
    {
      CORBA_exception_set_system(ev,
				 ex_CORBA_BAD_PARAM,
				 CORBA_COMPLETED_NO);
      return;
    }

  if(_obj->state == PortableServer_POAManager_INACTIVE)
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_POAManager_AdapterInactive,
			  NULL);
      return;
    }

  if ( wait_for_completion )
    {
      for (poai = _obj->poa_collection; poai; poai = poai->next)
	{
	  if ( !ORBit_POA_is_inuse(poai->data, /*kids*/0, ev) )
	    {
	      CORBA_exception_set_system(ev, 
					 ex_CORBA_BAD_INV_ORDER,
					 CORBA_COMPLETED_NO);
	      return;
	    }
	}
    }

  _obj->state = PortableServer_POAManager_INACTIVE;

  for (poai = _obj->poa_collection; poai; poai = poai->next)
    ORBit_POA_deactivate(poai->data, etherealize_objects, ev);
}

PortableServer_POAManager_State
PortableServer_POAManager_get_state(PortableServer_POAManager _obj,
				    CORBA_Environment * ev)
{
  return _obj->state;
}

