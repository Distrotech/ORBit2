#include <config.h>
#include <orbit/orbit.h>

#define POLICY__get_value(name)                                              \
	name##Value                                                          \
	name##__get_value (name policy,                                      \
			   CORBA_Environment *ev)                            \
	{                                                                    \
		return ORBit_Policy_get ((CORBA_Policy)policy);              \
	}

POLICY__get_value (PortableServer_ThreadPolicy)
POLICY__get_value (PortableServer_LifespanPolicy)
POLICY__get_value (PortableServer_IdUniquenessPolicy)
POLICY__get_value (PortableServer_IdAssignmentPolicy)
POLICY__get_value (PortableServer_ServantRetentionPolicy)
POLICY__get_value (PortableServer_RequestProcessingPolicy)
POLICY__get_value (PortableServer_ImplicitActivationPolicy)

#define PortableServer_POA_create_POLICY(name, str, id)                      \
	name                                                                 \
	PortableServer_POA_create_##str##_policy (PortableServer_POA  poa,   \
						  const name##Value   value, \
						  CORBA_Environment  *ev)    \
	{                                                                    \
		return (name)ORBit_Policy_new (id, value);                   \
	}

PortableServer_POA_create_POLICY (PortableServer_ThreadPolicy, thread,
				  PortableServer_THREAD_POLICY_ID)

PortableServer_POA_create_POLICY (PortableServer_LifespanPolicy, lifespan,
				  PortableServer_LIFESPAN_POLICY_ID)

PortableServer_POA_create_POLICY (PortableServer_IdUniquenessPolicy, 
				  id_uniqueness,
				  PortableServer_ID_UNIQUENESS_POLICY_ID)

PortableServer_POA_create_POLICY (PortableServer_IdAssignmentPolicy, 
				  id_assignment,
				  PortableServer_ID_ASSIGNMENT_POLICY_ID)

PortableServer_POA_create_POLICY (PortableServer_ServantRetentionPolicy, 
				  servant_retention,
				  PortableServer_SERVANT_RETENTION_POLICY_ID)

PortableServer_POA_create_POLICY (PortableServer_RequestProcessingPolicy, 
				  request_processing,
				  PortableServer_REQUEST_PROCESSING_POLICY_ID)

PortableServer_POA_create_POLICY (PortableServer_ImplicitActivationPolicy, 
				  implicit_activation,
				  PortableServer_IMPLICIT_ACTIVATION_POLICY_ID)

ORBit_PortableServer_OkeyrandPolicy
PortableServer_POA_create_okeyrand_policy (PortableServer_POA         poa,
					   const CORBA_unsigned_long  poa_rand_len,
					   CORBA_Environment         *ev)
{
	return (ORBit_PortableServer_OkeyrandPolicy)
			ORBit_Policy_new (ORBit_PortableServer_OKEYRAND_POLICY_ID,
                                          poa_rand_len);
}
