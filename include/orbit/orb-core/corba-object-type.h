#ifndef CORBA_OBJECT_TYPE_H
#define CORBA_OBJECT_TYPE_H 1

typedef struct {
  CORBA_sequence_CORBA_octet object_key;
  struct iovec object_key_vec;
  struct { CORBA_unsigned_long _length; char _buffer[1]; } object_key_data;
} IOP_ObjectKey_info;

struct CORBA_Object_type {
  struct ORBit_RootObject_struct parent;
  CORBA_ORB orb;
  GIOPConnection *connection;
  CORBA_char *type_id;
  GSList *profile_list, *forward_locations;
  IOP_ObjectKey_info *oki;

  ORBit_OAObject adaptor_obj;
};

#endif
