#ifndef CORBA_OBJECT_TYPE_H
#define CORBA_OBJECT_TYPE_H 1

typedef CORBA_sequence_CORBA_octet ORBit_ObjectKey;

struct CORBA_Object_type {
	struct ORBit_RootObject_struct parent;

	CORBA_ORB                      orb;
	GIOPConnection                *connection;
	CORBA_char                    *type_id;
	GSList                        *profile_list;
	GSList                        *forward_locations;
	ORBit_ObjectKey               *object_key;
	ORBit_OAObject                 adaptor_obj;
};

#endif
