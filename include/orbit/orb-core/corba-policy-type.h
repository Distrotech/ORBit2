#ifndef CORBA_POLICY_TYPE_H
#define CORBA_POLICY_TYPE_H 1

#ifdef ORBIT2_INTERNAL_API

struct CORBA_Policy_type {
	struct ORBit_RootObject_struct parent;

	CORBA_unsigned_long            type;
	CORBA_unsigned_long            value; 
};

CORBA_Policy        ORBit_Policy_new (CORBA_unsigned_long type,
				      CORBA_unsigned_long value);
void                ORBit_Policy_set (CORBA_Policy p, CORBA_unsigned_long value);
CORBA_unsigned_long ORBit_Policy_get (CORBA_Policy p);

#endif /* ORBIT2_INTERNAL_API */

#endif
