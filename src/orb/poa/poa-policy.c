#include <orbit/orbit.h>

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
  return (PortableServer_LifespanPolicy)ORBit_Policy_new(PortableServer_LIFESPAN_POLICY_ID, value);
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

ORBit_PortableServer_OkeyrandPolicy
PortableServer_POA_create_okeyrand_policy (PortableServer_POA         poa,
					   const CORBA_unsigned_long  poa_rand_len,
					   CORBA_Environment         *ev)
{
	return (ORBit_PortableServer_OkeyrandPolicy)
			ORBit_Policy_new (ORBit_PortableServer_OKEYRAND_POLICY_ID,
                                          poa_rand_len);
}
