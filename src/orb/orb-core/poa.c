#include <orbit/orbit.h>

PortableServer_ThreadPolicyValue
PortableServer_ThreadPolicy__get_value(PortableServer_ThreadPolicy _obj,
				       CORBA_Environment * ev)
{
}

PortableServer_LifespanPolicyValue
PortableServer_LifespanPolicy__get_value(PortableServer_LifespanPolicy _obj,
					 CORBA_Environment * ev)
{
}

PortableServer_IdUniquenessPolicyValue
PortableServer_IdUniquenessPolicy__get_value
(PortableServer_IdUniquenessPolicy _obj, CORBA_Environment * ev)
{
}

PortableServer_IdAssignmentPolicyValue
PortableServer_IdAssignmentPolicy__get_value
(PortableServer_IdAssignmentPolicy _obj, CORBA_Environment * ev)
{
}

PortableServer_ImplicitActivationPolicyValue
PortableServer_ImplicitActivationPolicy__get_value
(PortableServer_ImplicitActivationPolicy _obj, CORBA_Environment * ev)
{
}

PortableServer_ServantRetentionPolicyValue
PortableServer_ServantRetentionPolicy__get_value
(PortableServer_ServantRetentionPolicy _obj, CORBA_Environment * ev)
{
}

PortableServer_RequestProcessingPolicyValue
PortableServer_RequestProcessingPolicy__get_value
(PortableServer_RequestProcessingPolicy _obj, CORBA_Environment * ev)
{
}

void
PortableServer_POAManager_activate(PortableServer_POAManager _obj,
				   CORBA_Environment * ev)
{
}

void
PortableServer_POAManager_hold_requests(PortableServer_POAManager _obj,
					const CORBA_boolean wait_for_completion,
					CORBA_Environment * ev)
{
}

void
PortableServer_POAManager_discard_requests(PortableServer_POAManager _obj,
					   const CORBA_boolean wait_for_completion,
					   CORBA_Environment * ev)
{
}

void
PortableServer_POAManager_deactivate(PortableServer_POAManager _obj,
				     const CORBA_boolean etherealize_objects,
				     const CORBA_boolean wait_for_completion,
				     CORBA_Environment * ev)
{
}

PortableServer_POAManager_State
PortableServer_POAManager_get_state(PortableServer_POAManager _obj,
				    CORBA_Environment * ev)
{
}

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

PortableServer_POA
PortableServer_POA_create_POA(PortableServer_POA _obj,
			      const CORBA_char *adapter_name,
			      const PortableServer_POAManager a_POAManager,
			      const CORBA_PolicyList *policies,
			      CORBA_Environment * ev)
{
}

PortableServer_POA
PortableServer_POA_find_POA(PortableServer_POA _obj,
			    const CORBA_char *adapter_name,
			    const CORBA_boolean activate_it,
			    CORBA_Environment * ev)
{
}

void
PortableServer_POA_destroy(PortableServer_POA _obj,
			   const CORBA_boolean etherealize_objects,
			   const CORBA_boolean wait_for_completion,
			   CORBA_Environment * ev)
{
}

PortableServer_ThreadPolicy
PortableServer_POA_create_thread_policy
(PortableServer_POA _obj,
 const PortableServer_ThreadPolicyValue value,
 CORBA_Environment * ev)
{
}

PortableServer_LifespanPolicy
PortableServer_POA_create_lifespan_policy(PortableServer_POA _obj,
					  const PortableServer_LifespanPolicyValue value,
					  CORBA_Environment * ev)
{
}

PortableServer_IdUniquenessPolicy
PortableServer_POA_create_id_uniqueness_policy(PortableServer_POA _obj,
					       const PortableServer_IdUniquenessPolicyValue value,
					       CORBA_Environment * ev)
{
}

PortableServer_IdAssignmentPolicy
PortableServer_POA_create_id_assignment_policy(PortableServer_POA _obj,
					       const PortableServer_IdAssignmentPolicyValue value,
					       CORBA_Environment * ev)
{
}

PortableServer_ImplicitActivationPolicy
PortableServer_POA_create_implicit_activation_policy(PortableServer_POA _obj,
						     const PortableServer_ImplicitActivationPolicyValue value,
						     CORBA_Environment *ev)
{
}

PortableServer_ServantRetentionPolicy
PortableServer_POA_create_servant_retention_policy(PortableServer_POA _obj,
						   const PortableServer_ServantRetentionPolicyValue value,
						   CORBA_Environment *ev)
{
}

PortableServer_RequestProcessingPolicy
PortableServer_POA_create_request_processing_policy(PortableServer_POA _obj,
						    const PortableServer_RequestProcessingPolicyValue value,
						    CORBA_Environment *ev)
{
}

CORBA_string
PortableServer_POA__get_the_name(PortableServer_POA _obj,
				 CORBA_Environment * ev)
{
}

PortableServer_POA
PortableServer_POA__get_the_parent(PortableServer_POA _obj,
				   CORBA_Environment *ev)
{
}

PortableServer_POAList *
PortableServer_POA__get_the_children(PortableServer_POA _obj,
				     CORBA_Environment * ev)
{
}

PortableServer_POAManager
PortableServer_POA__get_the_POAManager(PortableServer_POA _obj,
				       CORBA_Environment * ev)
{
}

PortableServer_AdapterActivator
PortableServer_POA__get_the_activator(PortableServer_POA _obj,
				      CORBA_Environment * ev)
{
}

void
PortableServer_POA__set_the_activator(PortableServer_POA _obj,
				      const PortableServer_AdapterActivator value,
				      CORBA_Environment * ev)
{
}

PortableServer_ServantManager
PortableServer_POA_get_servant_manager(PortableServer_POA _obj,
				       CORBA_Environment * ev)
{
}

void PortableServer_POA_set_servant_manager(PortableServer_POA _obj,
					    const PortableServer_ServantManager imgr,
					    CORBA_Environment * ev)
{
}

PortableServer_Servant
PortableServer_POA_get_servant(PortableServer_POA _obj,
			      CORBA_Environment *ev)
{
}

void
PortableServer_POA_set_servant(PortableServer_POA _obj,
			       const PortableServer_Servant p_servant,
			       CORBA_Environment * ev)
{
}

PortableServer_ObjectId*
PortableServer_POA_activate_object(PortableServer_POA _obj,
				   const PortableServer_Servant p_servant,
				   CORBA_Environment * ev)
{
}

void
PortableServer_POA_activate_object_with_id(PortableServer_POA _obj,
					   const PortableServer_ObjectId *id,
					   const PortableServer_Servant p_servant,
					   CORBA_Environment * ev)
{
}

void
PortableServer_POA_deactivate_object(PortableServer_POA _obj,
				     const PortableServer_ObjectId *oid,
				     CORBA_Environment * ev)
{
}

CORBA_Object
PortableServer_POA_create_reference(PortableServer_POA _obj,
				    const CORBA_RepositoryId intf,
				    CORBA_Environment * ev)
{
}

CORBA_Object
PortableServer_POA_create_reference_with_id(PortableServer_POA _obj,
					    const PortableServer_ObjectId* oid,
					    const CORBA_RepositoryId intf,
					    CORBA_Environment* ev)
{
}

PortableServer_ObjectId *
PortableServer_POA_servant_to_id(PortableServer_POA _obj,
				 const PortableServer_Servant p_servant,
				 CORBA_Environment * ev)
{
}

CORBA_Object
PortableServer_POA_servant_to_reference(PortableServer_POA _obj,
					const PortableServer_Servant p_servant,
					CORBA_Environment *ev)
{
}

PortableServer_Servant
PortableServer_POA_reference_to_servant(PortableServer_POA _obj,
					const CORBA_Object reference,
					CORBA_Environment * ev)
{
}

PortableServer_ObjectId *
PortableServer_POA_reference_to_id(PortableServer_POA _obj,
				   const CORBA_Object reference,
				   CORBA_Environment * ev)
{
}

PortableServer_Servant
PortableServer_POA_id_to_servant(PortableServer_POA _obj,
				 const PortableServer_ObjectId* oid,
				 CORBA_Environment *ev)
{
}

CORBA_Object
PortableServer_POA_id_to_reference(PortableServer_POA _obj,
				   const PortableServer_ObjectId *oid,
				   CORBA_Environment * ev)
{
}

PortableServer_POA
PortableServer_Current_get_POA(PortableServer_Current _obj,
			       CORBA_Environment * ev)
{
}

PortableServer_ObjectId *
PortableServer_Current_get_object_id(PortableServer_Current _obj,
				     CORBA_Environment * ev)
{
}
