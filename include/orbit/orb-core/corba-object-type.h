#ifndef CORBA_OBJECT_TYPE_H
#define CORBA_OBJECT_TYPE_H 1

#if 0
typedef gpointer ORBit_POAObject;

typedef struct {
  CORBA_octet iiop_major, iiop_minor;

  IOP_ProfileId profile_type;
  
  CORBA_sequence_octet object_key;
  struct { CORBA_unsigned_long _length; char _buffer[1]; } *object_key_data;
  struct iovec object_key_vec;
} IOP_Profile_info;

struct CORBA_Object_type {
  struct ORBit_RootObject_struct parent;
  CORBA_ORB orb;
  GIOPConnection *active_connection;
  CORBA_char *type_id;
  GSList *profile_list, *forward_locations;
  IOP_Profile_info *active_profile;

  ORBit_POAObject bypass_obj;
};
#endif

#endif
