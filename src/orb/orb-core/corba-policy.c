#include "config.h"
#include <orbit/orbit.h>

CORBA_Policy
ORBit_Policy_new(CORBA_unsigned_long type,
		 CORBA_unsigned_long value)
{
  static ORBit_RootObject_Interface policy_if = {
    ORBIT_ROT_POLICY, NULL
  };
  CORBA_Policy_t retval;
  retval = g_new0(struct CORBA_Policy_type, 1);
  ORBit_RootObject_init(ORBIT_ROOT_OBJECT(retval), &policy_if);
  retval->type = type;
  retval->value = value;

  return (CORBA_Policy)retval;
}

void
ORBit_Policy_set(CORBA_Policy p, CORBA_unsigned_long value)
{
  CORBA_Policy_t pt = POLICY_CAST_IN(p);
  pt->value = value;
}

CORBA_unsigned_long
ORBit_Policy_get(CORBA_Policy p)
{
  CORBA_Policy_t pt = POLICY_CAST_IN(p);
  return pt->value;
}

CORBA_PolicyType
CORBA_Policy__get_policy_type(CORBA_Policy _obj,
			      CORBA_Environment * ev)
{
  CORBA_Policy_t pt = POLICY_CAST_IN(_obj);
  return pt->type;
}

CORBA_Policy
CORBA_Policy_copy(CORBA_Policy _obj, CORBA_Environment * ev)
{
  CORBA_Policy_t pt = POLICY_CAST_IN(_obj);
  return CORBA_Object_duplicate(POLICY_CAST_OUT(ORBit_Policy_new(pt->type,
								 pt->value)),
				ev);
}

void
CORBA_Policy_destroy(CORBA_Policy _obj, CORBA_Environment * ev)
{
  /* Do nothing */
}

CORBA_Policy
CORBA_DomainManager_get_domain_policy(CORBA_DomainManager _obj,
				      const CORBA_PolicyType policy_type,
				      CORBA_Environment * ev)
{
  return NULL;
}

void
CORBA_ConstructionPolicy_make_domain_manager(CORBA_ConstructionPolicy _obj,
					     const CORBA_InterfaceDef object_type,
					     const CORBA_boolean constr_policy,
					     CORBA_Environment * ev)
{
}
