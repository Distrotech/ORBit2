#ifndef __IOP_PROFILES_H__
#define __IOP_PROFILES_H__ 1

typedef struct {
	IOP_ProfileId               profile_type;
} IOP_Profile_info;

typedef struct {
	IOP_Profile_info            parent;

	GIOPVersion                 iiop_version;
	gchar                      *host;
	CORBA_unsigned_short        port;
	ORBit_ObjectKey            *object_key;
	GSList                     *components;
} IOP_TAG_INTERNET_IOP_info;

typedef struct {
	IOP_Profile_info            parent;

	GIOPVersion                 iiop_version;
	gchar                      *proto;
	gchar                      *host;
	gchar                      *service;
	GSList                     *components;
} IOP_TAG_GENERIC_IOP_info;

typedef struct {
	IOP_Profile_info            parent;

	gchar                      *unix_sock_path;
	CORBA_unsigned_short        ipv6_port;
	ORBit_ObjectKey            *object_key;
} IOP_TAG_ORBIT_SPECIFIC_info;

typedef struct {
	IOP_Profile_info            parent;

	GSList                     *components;
} IOP_TAG_MULTIPLE_COMPONENTS_info;

typedef struct {
	IOP_Profile_info            parent;

	CORBA_sequence_CORBA_octet  data;
} IOP_UnknownProfile_info;

typedef struct {
	IOP_ComponentId             component_type;
} IOP_Component_info;

typedef struct {
	IOP_Component_info          parent;

	gchar                      *service;
} IOP_TAG_GENERIC_SSL_SEC_TRANS_info;

typedef struct {
	IOP_Component_info          parent;

	CORBA_unsigned_long         target_supports;
	CORBA_unsigned_long         target_requires;
	CORBA_unsigned_short        port;
} IOP_TAG_SSL_SEC_TRANS_info;

typedef struct {
	IOP_Component_info          parent;

	ORBit_ObjectKey            *object_key;
} IOP_TAG_COMPLETE_OBJECT_KEY_info;

typedef struct {
	IOP_Component_info          parent;
} IOP_TAG_CODE_SETS_info;

typedef struct {
	IOP_Component_info          parent;

	CORBA_sequence_CORBA_octet  data;
} IOP_UnknownComponent_info;

#endif /* __IOP_PROFILES_H__ */
