#include <orbit/orbit.h>
#include "orbit-poa.h"
#include <string.h>

PortableServer_ThreadPolicyValue
PortableServer_ThreadPolicy__get_value(PortableServer_ThreadPolicy _obj,
				       CORBA_Environment * ev)
{
  return ORBit_Policy_get((CORBA_Policy)_obj);
}

PortableServer_LifespanPolicyValue
PortableServer_LifespanPolicy__get_value(PortableServer_LifespanPolicy _obj,
					 CORBA_Environment * ev)
{
  return ORBit_Policy_get((CORBA_Policy)_obj);
}

PortableServer_IdUniquenessPolicyValue
PortableServer_IdUniquenessPolicy__get_value
(PortableServer_IdUniquenessPolicy _obj, CORBA_Environment * ev)
{
  return ORBit_Policy_get((CORBA_Policy)_obj);
}

PortableServer_IdAssignmentPolicyValue
PortableServer_IdAssignmentPolicy__get_value
(PortableServer_IdAssignmentPolicy _obj, CORBA_Environment * ev)
{
  return ORBit_Policy_get((CORBA_Policy)_obj);
}

PortableServer_ImplicitActivationPolicyValue
PortableServer_ImplicitActivationPolicy__get_value
(PortableServer_ImplicitActivationPolicy _obj, CORBA_Environment * ev)
{
  return ORBit_Policy_get((CORBA_Policy)_obj);
}

PortableServer_ServantRetentionPolicyValue
PortableServer_ServantRetentionPolicy__get_value
(PortableServer_ServantRetentionPolicy _obj, CORBA_Environment * ev)
{
  return ORBit_Policy_get((CORBA_Policy)_obj);
}

PortableServer_RequestProcessingPolicyValue
PortableServer_RequestProcessingPolicy__get_value
(PortableServer_RequestProcessingPolicy _obj, CORBA_Environment * ev)
{
  return ORBit_Policy_get((CORBA_Policy)_obj);
}

PortableServer_POAManager
ORBit_POAManager_new(CORBA_ORB orb, CORBA_Environment *ev)
{
  PortableServer_POAManager retval;
  retval = g_new0(struct PortableServer_POAManager_type, 1);

  return retval;
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

PortableServer_POA
PortableServer_POA_create_POA(PortableServer_POA _obj,
			      const CORBA_char *adapter_name,
			      const PortableServer_POAManager a_POAManager,
			      const CORBA_PolicyList *policies,
			      CORBA_Environment * ev)
{
  PortableServer_POA new_poa = NULL;
  PortableServer_POA check_poa = NULL;
	
  /* Check for a child POA by the same name in parent */
  check_poa = PortableServer_POA_find_POA(_obj, adapter_name,
					  FALSE, ev);
  CORBA_exception_free (ev);

  if (!check_poa)
    new_poa = ORBit_POA_new(_obj->orb,
			    adapter_name, a_POAManager, policies, ev);
  else {
    CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			ex_PortableServer_POA_AdapterAlreadyExists,
			NULL);
    new_poa = NULL;
  }

  if(ev->_major == CORBA_NO_EXCEPTION)
    /* register parent-child (it will do both cross-link pointers) */
    ORBit_POA_add_child(_obj, new_poa, ev);

  return new_poa;
}

PortableServer_POA
PortableServer_POA_find_POA(PortableServer_POA _obj,
			    const CORBA_char *adapter_name,
			    const CORBA_boolean activate_it,
			    CORBA_Environment * ev)
{
  PortableServer_POA child_poa;


  child_poa = g_hash_table_lookup(_obj->child_poas, adapter_name);

  if(activate_it)
    g_warning("Don't yet know how to activate POA named \"%s\"",
	      adapter_name);

  if(child_poa)
    child_poa = (PortableServer_POA)
      CORBA_Object_duplicate((CORBA_Object)child_poa, ev);
  else
    CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			ex_PortableServer_POA_AdapterNonExistent,
			NULL);	

  return child_poa;
}

void
PortableServer_POA_destroy(PortableServer_POA _obj,
			   const CORBA_boolean etherealize_objects,
			   const CORBA_boolean wait_for_completion,
			   CORBA_Environment * ev)
{
  CORBA_boolean	done;

  if ( _obj->life_flags & ORBit_LifeF_Destroyed )
    return;
  if ( wait_for_completion && ORBit_POA_is_inuse(_obj, /*kids*/1, ev) )
    {
      CORBA_exception_set_system(ev, ex_CORBA_BAD_INV_ORDER,
				 CORBA_COMPLETED_NO);
      return;
    }
  done = ORBit_POA_destroy(_obj, etherealize_objects, ev);
  g_assert( done || !wait_for_completion );
}

PortableServer_ThreadPolicy
PortableServer_POA_create_thread_policy(PortableServer_POA _obj,
					const PortableServer_ThreadPolicyValue value,
					CORBA_Environment * ev)
{
  return (PortableServer_ThreadPolicy)ORBit_Policy_new(PortableServer_THREAD_POLICY_ID, value);
}

PortableServer_LifespanPolicy
PortableServer_POA_create_lifespan_policy(PortableServer_POA _obj,
					  const PortableServer_LifespanPolicyValue value,
					  CORBA_Environment * ev)
{
  return (PortableServer_LifespanPolicy)ORBit_Policy_new(PortableServer_THREAD_POLICY_ID, value);
}

PortableServer_IdUniquenessPolicy
PortableServer_POA_create_id_uniqueness_policy(PortableServer_POA _obj,
					       const PortableServer_IdUniquenessPolicyValue value,
					       CORBA_Environment * ev)
{
  return (PortableServer_IdUniquenessPolicy)ORBit_Policy_new(PortableServer_ID_UNIQUENESS_POLICY_ID, value);
}

PortableServer_IdAssignmentPolicy
PortableServer_POA_create_id_assignment_policy(PortableServer_POA _obj,
					       const PortableServer_IdAssignmentPolicyValue value,
					       CORBA_Environment * ev)
{
  return (PortableServer_IdAssignmentPolicy)ORBit_Policy_new(PortableServer_ID_ASSIGNMENT_POLICY_ID, value);
}

PortableServer_ImplicitActivationPolicy
PortableServer_POA_create_implicit_activation_policy(PortableServer_POA _obj,
						     const PortableServer_ImplicitActivationPolicyValue value,
						     CORBA_Environment *ev)
{
  return (PortableServer_ImplicitActivationPolicy)ORBit_Policy_new(PortableServer_IMPLICIT_ACTIVATION_POLICY_ID, value);
}

PortableServer_ServantRetentionPolicy
PortableServer_POA_create_servant_retention_policy(PortableServer_POA _obj,
						   const PortableServer_ServantRetentionPolicyValue value,
						   CORBA_Environment *ev)
{
  return (PortableServer_ServantRetentionPolicy)ORBit_Policy_new(PortableServer_SERVANT_RETENTION_POLICY_ID, value);
}

PortableServer_RequestProcessingPolicy
PortableServer_POA_create_request_processing_policy(PortableServer_POA _obj,
						    const PortableServer_RequestProcessingPolicyValue value,
						    CORBA_Environment *ev)
{
  return (PortableServer_RequestProcessingPolicy)ORBit_Policy_new(PortableServer_REQUEST_PROCESSING_POLICY_ID, value);
}

CORBA_string
PortableServer_POA__get_the_name(PortableServer_POA _obj,
				 CORBA_Environment * ev)
{
  return CORBA_string_dup(_obj->name);
}

PortableServer_POA
PortableServer_POA__get_the_parent(PortableServer_POA _obj,
				   CORBA_Environment *ev)
{
  return (PortableServer_POA)
    CORBA_Object_duplicate((CORBA_Object)_obj->parent_poa, ev);
}

static void
poalist_add_child(gpointer key, gpointer value, gpointer data)
{
  PortableServer_POAList *retval = data;

  retval->_buffer[retval->_maximum++] =
    (PortableServer_POA)CORBA_Object_duplicate(value, NULL);
}

PortableServer_POAList *
PortableServer_POA__get_the_children(PortableServer_POA _obj,
				     CORBA_Environment * ev)
{
  PortableServer_POAList *retval;

  retval = PortableServer_POAList__alloc();
  retval->_length = g_hash_table_size(_obj->child_poas);
  retval->_maximum = 0;
  retval->_buffer = CORBA_sequence_PortableServer_POA_allocbuf(retval->_length);
  g_hash_table_foreach(_obj->child_poas, poalist_add_child, retval);

  return retval;
}

PortableServer_POAManager
PortableServer_POA__get_the_POAManager(PortableServer_POA _obj,
				       CORBA_Environment * ev)
{
  return (PortableServer_POAManager)
    CORBA_Object_duplicate((CORBA_Object)_obj->poa_manager, ev);
}

PortableServer_AdapterActivator
PortableServer_POA__get_the_activator(PortableServer_POA _obj,
				      CORBA_Environment * ev)
{
  return (PortableServer_AdapterActivator)
    CORBA_Object_duplicate((CORBA_Object)_obj->the_activator, ev);
}

void
PortableServer_POA__set_the_activator(PortableServer_POA _obj,
				      const PortableServer_AdapterActivator value,
				      CORBA_Environment * ev)
{
  if(_obj->the_activator)
    CORBA_Object_release((CORBA_Object)_obj->the_activator, ev);
  _obj->the_activator = (PortableServer_AdapterActivator)
    CORBA_Object_duplicate((CORBA_Object)value, ev);
}

PortableServer_ServantManager
PortableServer_POA_get_servant_manager(PortableServer_POA _obj,
				       CORBA_Environment * ev)
{
  return (PortableServer_ServantManager)
    CORBA_Object_duplicate((CORBA_Object)_obj->servant_manager, ev);
}

void
PortableServer_POA_set_servant_manager(PortableServer_POA _obj,
				       const PortableServer_ServantManager imgr,
				       CORBA_Environment * ev)
{
  if(_obj->servant_manager)
    CORBA_Object_release((CORBA_Object)_obj->servant_manager, ev);
  _obj->servant_manager = (PortableServer_ServantManager)
    CORBA_Object_duplicate((CORBA_Object)imgr, ev);
}

PortableServer_Servant
PortableServer_POA_get_servant(PortableServer_POA _obj,
			       CORBA_Environment *ev)
{
  if(_obj->default_pobj)
    return _obj->default_pobj->servant;
  else
    return NULL;
}

void
PortableServer_POA_set_servant(PortableServer_POA _obj,
			       const PortableServer_Servant p_servant,
			       CORBA_Environment * ev)
{
  if(p_servant)
    _obj->default_pobj = ((PortableServer_ServantBase *)p_servant)->_private;
  else
    _obj->default_pobj = NULL;
}

PortableServer_ObjectId*
PortableServer_POA_activate_object(PortableServer_POA _obj,
				   const PortableServer_Servant p_servant,
				   CORBA_Environment * ev)
{
  g_error("NYI");

  return NULL;
}

void
PortableServer_POA_activate_object_with_id(PortableServer_POA _obj,
					   const PortableServer_ObjectId *id,
					   const PortableServer_Servant p_servant,
					   CORBA_Environment * ev)
{
  g_error("NYI");
}

void
PortableServer_POA_deactivate_object(PortableServer_POA _obj,
				     const PortableServer_ObjectId *oid,
				     CORBA_Environment * ev)
{
  g_error("NYI");
}

CORBA_Object
PortableServer_POA_create_reference(PortableServer_POA _obj,
				    const CORBA_RepositoryId intf,
				    CORBA_Environment * ev)
{
  CORBA_Object retval;

  if ( _obj->p_id_assignment != PortableServer_SYSTEM_ID )
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_POA_WrongPolicy, NULL);
      return NULL;
    }

  if( _obj->p_servant_retention == PortableServer_RETAIN )
    {
      ORBit_POAObject	*pobj;

      pobj = ORBit_POA_create_object(_obj, /*oid*/NULL, /*isDef*/0, ev);
      retval = ORBit_POA_obj_to_ref(_obj, pobj, intf, ev);
    }
  else
    {
      PortableServer_ObjectId oid;

      ORBit_POA_make_sysoid(_obj, &oid);
      retval = ORBit_POA_oid_to_ref(_obj, &oid, intf, ev);
    }

  return retval;
}

CORBA_Object
PortableServer_POA_create_reference_with_id(PortableServer_POA _obj,
					    const PortableServer_ObjectId* oid,
					    const CORBA_RepositoryId intf,
					    CORBA_Environment* ev)
{
  if ( _obj->p_servant_retention == PortableServer_RETAIN )
    {
      ORBit_POAObject	*pobj;
      pobj = ORBit_POA_oid_to_obj(_obj, oid, /*active*/0, /*ev*/0);
      if ( pobj == NULL )
	pobj = ORBit_POA_create_object(_obj, oid, /*isDef*/0, ev);
      return ORBit_POA_obj_to_ref(_obj, pobj, intf, ev);
    }
  return ORBit_POA_oid_to_ref(_obj, oid, intf, ev);
}

PortableServer_ObjectId *
PortableServer_POA_servant_to_id(PortableServer_POA _obj,
				 const PortableServer_Servant p_servant,
				 CORBA_Environment * ev)
{
  return NULL;
}

CORBA_Object
PortableServer_POA_servant_to_reference(PortableServer_POA _obj,
					const PortableServer_Servant p_servant,
					CORBA_Environment *ev)
{
  return NULL;
}

PortableServer_Servant
PortableServer_POA_reference_to_servant(PortableServer_POA _obj,
					const CORBA_Object reference,
					CORBA_Environment * ev)
{
  return NULL;
}

PortableServer_ObjectId *
PortableServer_POA_reference_to_id(PortableServer_POA _obj,
				   const CORBA_Object reference,
				   CORBA_Environment * ev)
{
  return NULL;
}

PortableServer_Servant
PortableServer_POA_id_to_servant(PortableServer_POA _obj,
				 const PortableServer_ObjectId* oid,
				 CORBA_Environment *ev)
{
  return NULL;
}

CORBA_Object
PortableServer_POA_id_to_reference(PortableServer_POA _obj,
				   const PortableServer_ObjectId *oid,
				   CORBA_Environment * ev)
{
  return NULL;
}

/* PortableServer_Current interface */
static void
ORBit_POACurrent_free_fn(ORBit_RootObject obj_in)
{
    PortableServer_Current poacur = (PortableServer_Current)obj_in;
    ORBit_RootObject_release(poacur->orb);
    g_free(poacur);
}

static const ORBit_RootObject_Interface ORBit_POACurrent_epv = {
    ORBIT_ROT_POACURRENT,
    ORBit_POACurrent_free_fn
};

/**
    The returned object has already been dup'd; caller must free
    if it doesnt want it!
**/
PortableServer_Current
ORBit_POACurrent_new(CORBA_ORB orb)
{
    PortableServer_Current poacur;

    poacur = (PortableServer_Current) 
      g_new0(struct PortableServer_Current_type, 1);
    ORBit_RootObject_init(&poacur->parent, &ORBit_POACurrent_epv);
    poacur->orb = ORBit_RootObject_duplicate(orb);
    return ORBit_RootObject_duplicate(poacur);
}

static ORBit_POAInvocation*
ORBit_POACurrent_get_invocation(PortableServer_Current obj,
				CORBA_Environment *ev)
{
  g_assert( obj && obj->parent.interface->type == ORBIT_ROT_POACURRENT );

  if ( obj->orb->poa_current_invocations == 0 )
    {
      CORBA_exception_set(ev, CORBA_USER_EXCEPTION,
			  ex_PortableServer_Current_NoContext,
			  NULL);
      return 0;
    }

  return obj->orb->poa_current_invocations;
}

PortableServer_POA
PortableServer_Current_get_POA(PortableServer_Current obj, CORBA_Environment *ev)
{
    ORBit_POAInvocation	*invoke;
    if ( (invoke=ORBit_POACurrent_get_invocation(obj, ev))==0 )
    	return 0;
    return ORBit_RootObject_duplicate(obj->orb->poa_current);
}

PortableServer_ObjectId *
PortableServer_Current_get_object_id(PortableServer_Current obj,
				     CORBA_Environment *ev)
{
    ORBit_POAInvocation	*invoke;

    if ( (invoke=ORBit_POACurrent_get_invocation(obj, ev))==0 )
	return 0;
    return
      (PortableServer_ObjectId *)
      ORBit_sequence_octet_dup(invoke->object_id 
      ? invoke->object_id : invoke->pobj->object_id);
}

/***********************************************************************
 *
 *		Object to/from strings
 *
 * These are specified in the C language mapping spec, not the base
 * spec. See sec 1.26.2. These are meanful only under USER_ID policy,
 * since SYSTEM_ID oids have NULs in them. Probably they would only
 * be used with MULTIPLE_ID/DEFAULT_SERVANT policies.
 * Using strings as oids may be convenient, but it seems horribly slow.
 *
 ***********************************************************************/

CORBA_char *
PortableServer_ObjectId_to_string(PortableServer_ObjectId *id, 
				  CORBA_Environment *ev)
{
  CORBA_char	*str;
  if ( memchr( id->_buffer, 0, id->_length) )
    {
      /* we could try to escape it, but the spec
       * specifically alllows us to throw an exception. */
      CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM,
				 CORBA_COMPLETED_NO);
      return NULL;
    }
  str = CORBA_string_alloc(id->_length);
  memcpy( str, id->_buffer, id->_length);
  str[id->_length] = 0;
  return str;
}

CORBA_wchar *
PortableServer_ObjectId_to_wstring(PortableServer_ObjectId *id,
				   CORBA_Environment *ev)
{
  CORBA_wchar *retval;
  int i;
  
  if ( memchr( id->_buffer, 0, id->_length) )
    {
      /* we could try to escape it, but the spec
       * specifically alllows us to throw an exception. */
      CORBA_exception_set_system(ev, ex_CORBA_BAD_PARAM,
				 CORBA_COMPLETED_NO);
      return NULL;
    }
  retval = CORBA_wstring_alloc(id->_length);
  for(i = 0; i < id->_length; i++)
    retval[i] = id->_buffer[i];
  retval[id->_length] = 0;
  return retval;
}

PortableServer_ObjectId *
PortableServer_string_to_ObjectId(CORBA_char *str, CORBA_Environment *env)
{
  CORBA_sequence_octet	tmp;
  tmp._length = strlen(str);
  tmp._buffer = str;
  return (PortableServer_ObjectId *)ORBit_sequence_octet_dup(&tmp);
}

PortableServer_ObjectId *
PortableServer_wstring_to_ObjectId(CORBA_wchar *str, CORBA_Environment *env)
{
  int i;
  CORBA_sequence_octet tmp;
  for(i = 0; str[i]; i++) /**/;
  tmp._length = i*2;
  tmp._buffer = g_alloca(tmp._length);
  for(i = 0; str[i]; i++)
    tmp._buffer[i] = str[i];

  return (PortableServer_ObjectId *)ORBit_sequence_octet_dup(&tmp);
}

PortableServer_POA
PortableServer_ServantBase__default_POA(PortableServer_Servant servant,
				       CORBA_Environment *ev)
{
  g_return_val_if_fail(servant, NULL);
  return ORBIT_SERVANT_TO_POAOBJECT(servant)->poa;
}

void
PortableServer_ServantBase__init(PortableServer_Servant p_servant,
				 CORBA_Environment *ev)
{
  PortableServer_ServantBase *servant = p_servant;
  /* NOTE: servant must be init'd before POAObj activated */
  g_assert( servant->_private == NULL );
}

void
PortableServer_ServantBase__fini(PortableServer_Servant p_servant,
				 CORBA_Environment *ev)
{
  /* NOTE: servant must be fini'd after POAObj de-activated */
  PortableServer_ServantBase *servant = p_servant;
  g_assert( servant->_private == NULL );
}

#if 0
CORBA_boolean
PortableServer_AdapterActivator_unknown_adapter
(PortableServer_AdapterActivator _obj,
 const PortableServer_POA parent,
 const CORBA_char * name,
 CORBA_Environment * ev)
{
}

PortableServer_Servant
PortableServer_ServantActivator_incarnate
(PortableServer_ServantActivator _obj,
 const PortableServer_ObjectId * oid,
 const PortableServer_POA adapter,
 CORBA_Environment * ev)
{
}

void
PortableServer_ServantActivator_etherealize
(PortableServer_ServantActivator _obj,
 const PortableServer_ObjectId * oid,
 const PortableServer_POA adapter,
 const PortableServer_Servant serv,
 const CORBA_boolean cleanup_in_progress,
 const CORBA_boolean remaining_activations,
 CORBA_Environment * ev)
{
}

PortableServer_Servant
PortableServer_ServantLocator_preinvoke(PortableServer_ServantLocator _obj,
					const PortableServer_ObjectId *oid,
					const PortableServer_POA adapter,
					const CORBA_Identifier operation,
					PortableServer_ServantLocator_Cookie the_cookie,
					CORBA_Environment * ev)
{
}

void
PortableServer_ServantLocator_postinvoke(PortableServer_ServantLocator _obj,
					 const PortableServer_ObjectId* oid,
					 const PortableServer_POA adapter,
					 const CORBA_Identifier operation,
					 const PortableServer_ServantLocator_Cookie the_cookie,
					 const PortableServer_Servant
					 the_servant,
					 CORBA_Environment * ev)
{
}
#endif

void
ORBit_classinfo_register(PortableServer_ClassInfo *ci)
{
}

gboolean
ORBit_POA_is_inuse(PortableServer_POA poa,
		   CORBA_boolean consider_children,
		   CORBA_Environment *ev)
{
  return FALSE;
}

ORBit_POAObject *ORBit_POA_create_object(PortableServer_POA poa,
					 const PortableServer_ObjectId *oid,
					 CORBA_boolean isDefault,
					 CORBA_Environment *ev)
{
  return NULL;
}

void
ORBit_POA_make_sysoid(PortableServer_POA poa, PortableServer_ObjectId *oid)
{
}

CORBA_Object
ORBit_POA_oid_to_ref(PortableServer_POA poa,
		     const PortableServer_ObjectId *oid,
		     const CORBA_RepositoryId intf,
		     CORBA_Environment *ev)
{
  return NULL;
}

ORBit_POAObject *
ORBit_POA_oid_to_obj(PortableServer_POA poa,
		     const PortableServer_ObjectId *oid,
		     gboolean active,
		     CORBA_Environment *ev)
{
  return NULL;
}

gboolean
ORBit_POA_destroy(PortableServer_POA poa,
		  CORBA_boolean etherealize_objects,
		  CORBA_Environment *ev)
{
  return FALSE;
}

void
ORBit_POA_add_child(PortableServer_POA poa,
		    PortableServer_POA child, 
		    CORBA_Environment *ev)
{
}

void
ORBit_POA_deactivate(PortableServer_POA poa,
		     CORBA_boolean etherealize_objects,
		     CORBA_Environment *ev)
{
}

void
ORBit_POA_handle_held_requests(PortableServer_POA poa)
{
}

static void
ORBit_POA_free_fn(ORBit_RootObject obj_in)
{
    PortableServer_POA poa = (PortableServer_POA)obj_in;
    ORBit_RootObject_release(poa->orb);
    g_free(poa);
}

static const ORBit_RootObject_Interface ORBit_POA_epv = {
    ORBIT_ROT_POA,
    ORBit_POA_free_fn
};


static guint
ORBit_ObjectId_sysid_hash(gconstpointer ptr)
{
  const PortableServer_ObjectId *oid = ptr;
    return *(guint*)oid->_buffer;
}

guint
ORBit_sequence_octet_hash(gconstpointer ptr)
{
  const CORBA_sequence_octet *so = ptr;
	const char *s = (char*)so->_buffer;
  	const char *p, *e = ((char *)so->_buffer) + so->_length;
  	guint h=0, g;

  	for(p = s; p < e; p ++) {
    		h = ( h << 4 ) + *p;
    		if ( ( g = h & 0xf0000000 ) ) {
      			h = h ^ (g >> 24);
      			h = h ^ g;
    		}
  	}
  	return h;
}

/*
 * Returns TRUE if s1 and s2 are the same, FALSE otherwise.
 * Note that this is what the glib hash module wants.
 */
gint
ORBit_sequence_octet_equal(gconstpointer p1, gconstpointer p2)
{
  const CORBA_sequence_octet *s1 = p1;
  const CORBA_sequence_octet *s2 = p2;
    int	same;
    same = s1->_length == s2->_length 
      && memcmp(s1->_buffer,s2->_buffer,s1->_length)==0;
    return same;
}

static void
ORBit_POA_check_policy_conflicts(PortableServer_POA poa, CORBA_Environment *ev)
{
}

static void
ORBit_POA_set_policylist(PortableServer_POA poa,
			 const CORBA_PolicyList *policies,
			 CORBA_Environment *ev)
{
}

PortableServer_POA
ORBit_POA_new(CORBA_ORB orb, const CORBA_char *nom,
	      PortableServer_POAManager manager,
	      const CORBA_PolicyList *policies,
	      CORBA_Environment *ev)
{
  PortableServer_POA poa;
  CORBA_long *tptr;
  
  poa = g_new0(struct PortableServer_POA_type, 1);
  ORBit_RootObject_init(&poa->parent, &ORBit_POA_epv);
  ORBit_RootObject_duplicate(poa);
  if(!manager)
    manager = ORBit_POAManager_new(orb, ev);
  if(ev->_major != CORBA_NO_EXCEPTION) goto error;
  poa->child_poas = NULL;
  poa->orb = ORBit_RootObject_duplicate(orb);
  poa->poaID = orb->poas->len;
  g_ptr_array_set_size(orb->poas, orb->poas->len + 1);
  g_ptr_array_index(orb->poas, poa->poaID) = poa;

  /* Need to initialise poa policies etc.. here */
  poa->p_thread = PortableServer_ORB_CTRL_MODEL;
  poa->p_lifespan = PortableServer_TRANSIENT;
  poa->p_id_uniqueness = PortableServer_UNIQUE_ID;
  poa->p_id_assignment = PortableServer_SYSTEM_ID;
  poa->p_servant_retention = PortableServer_RETAIN;
  poa->p_request_processing = PortableServer_USE_ACTIVE_OBJECT_MAP_ONLY;
  poa->p_implicit_activation = PortableServer_NO_IMPLICIT_ACTIVATION;

  if (policies) {
    ORBit_POA_set_policylist(poa, policies, ev);
    if(ev->_major != CORBA_NO_EXCEPTION) goto error;
  }
  /* check_policy also sets up some defaults, so need to always call */
  ORBit_POA_check_policy_conflicts(poa, ev);

  poa->poa_rand_len = 8;
  
  poa->name = CORBA_string_dup(nom);
  poa->poa_key._length = sizeof(CORBA_long) + poa->poa_rand_len;
  poa->poa_key._buffer =
    CORBA_sequence_CORBA_octet_allocbuf(poa->poa_key._length);
  tptr = (CORBA_long *)poa->poa_key._buffer;
  *tptr = poa->poaID;
  if(poa->poa_rand_len)
    ORBit_genrand_buf(&poa->orb->genrand,
		      poa->poa_key._buffer + sizeof(CORBA_long),
		      poa->poa_rand_len);
  if( poa->p_servant_retention == PortableServer_RETAIN )
    {
      if ( poa->p_id_assignment == PortableServer_SYSTEM_ID )
	poa->oid_to_obj_map = g_hash_table_new(ORBit_ObjectId_sysid_hash,
					       ORBit_sequence_octet_equal);
      else /* USER_ID */
	poa->oid_to_obj_map = g_hash_table_new(ORBit_sequence_octet_hash,
					       (GCompareFunc)ORBit_sequence_octet_equal);
    }

  if ( poa->p_id_assignment == PortableServer_USER_ID 
      && poa->obj_rand_len > 0 )
      {
	poa->num_to_koid_map = g_ptr_array_new();
	g_ptr_array_set_size(poa->num_to_koid_map, 1);
	g_ptr_array_index(poa->num_to_koid_map, 0) = NULL;
      }

  return poa;
error:
  g_free(poa); /* FIXME */
  return NULL;
}

CORBA_Object
ORBit_POA_obj_to_ref(PortableServer_POA poa,
		     ORBit_POAObject *pobj,
		     const CORBA_RepositoryId intf,
		     CORBA_Environment *ev)
{
  return NULL;
}

PortableServer_POA
ORBit_POA_setup_root(CORBA_ORB orb, CORBA_Environment *ev)
{
  return ORBit_POA_new(orb, "RootPOA", CORBA_OBJECT_NIL, NULL, ev);
}
