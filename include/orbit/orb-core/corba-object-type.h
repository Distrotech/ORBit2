#ifndef CORBA_OBJECT_TYPE_H
#define CORBA_OBJECT_TYPE_H 1

#if defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API)

#ifndef ORBIT2_INTERNAL_API
#define  GIOPConnection void
#endif

typedef CORBA_sequence_CORBA_octet ORBit_ObjectKey;

struct CORBA_Object_type {
	struct ORBit_RootObject_struct parent;

	CORBA_ORB                      orb;
	GIOPConnection                *connection;
	GQuark                         type_qid;
	GSList                        *profile_list;
	GSList                        *forward_locations;
	ORBit_ObjectKey               *object_key;
	ORBit_OAObject                 adaptor_obj;
};

#endif /* defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API) */

#endif
