#ifndef CORBA_OBJECT_TYPE_H
#define CORBA_OBJECT_TYPE_H 1

typedef struct {
  IOP_ProfileId profile_type;
} IOP_Profile_info;

typedef struct {
  CORBA_sequence_CORBA_octet object_key;
  struct iovec object_key_vec;
  struct { CORBA_unsigned_long _length; char _buffer[1]; } object_key_data;
} IOP_ObjectKey_info;

typedef struct {
  IOP_Profile_info parent;

  GIOPVersion iiop_version;
  char *host;
  CORBA_unsigned_short port;
  IOP_ObjectKey_info *oki;
  GSList *components;
} IOP_TAG_INTERNET_IOP_info;

typedef struct {
  IOP_Profile_info parent;

  GIOPVersion iiop_version;
  char *proto, *host, *service;
  GSList *components;
} IOP_TAG_GENERIC_IOP_info;

typedef struct {
  IOP_Profile_info parent;

  char *unix_sock_path;
  CORBA_unsigned_short ipv6_port;
  IOP_ObjectKey_info *oki;  
} IOP_TAG_ORBIT_SPECIFIC_info;

typedef struct {
  IOP_Profile_info parent;

  GSList *components;
} IOP_TAG_MULTIPLE_COMPONENTS_info;

typedef struct {
  IOP_Profile_info parent;

  CORBA_sequence_CORBA_octet data;
} IOP_UnknownProfile_info;

typedef struct {
  IOP_ComponentId component_type;
} IOP_Component_info;

typedef struct {
  IOP_Component_info parent;
  char *service;
} IOP_TAG_GENERIC_SSL_SEC_TRANS_info;

typedef struct {
  IOP_Component_info parent;
  CORBA_unsigned_long target_supports, target_requires;
  CORBA_unsigned_short port;
} IOP_TAG_SSL_SEC_TRANS_info;

typedef struct {
  IOP_Component_info parent;
  IOP_ObjectKey_info *oki;
} IOP_TAG_COMPLETE_OBJECT_KEY_info;

typedef struct {
  IOP_Component_info parent;

  CORBA_sequence_CORBA_octet data;
} IOP_UnknownComponent_info;

struct CORBA_Object_type {
  struct ORBit_RootObject_struct parent;
  CORBA_ORB orb;
  GIOPConnection *connection;
  CORBA_char *type_id;
  GSList *profile_list, *forward_locations;
  IOP_ObjectKey_info *oki; /* points into profile_list */

  ORBit_POAObject *pobj;
};

#endif
