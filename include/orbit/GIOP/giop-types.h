#ifndef GIOP_TYPES_H
#define GIOP_TYPES_H 1

#include <glib.h>
#include <glib-object.h>
#include <orbit/orbit-config.h>
#include <orbit/util/orbit-util.h>

typedef struct _GIOPRecvBuffer GIOPRecvBuffer;
typedef struct _GIOPSendBuffer GIOPSendBuffer;

typedef enum {
  GIOP_CONNECTION_SSL
} GIOPConnectionOptions;

typedef enum {
  GIOP_1_0,
  GIOP_1_1,
  GIOP_1_2,
  GIOP_LATEST = GIOP_1_2
} GIOPVersion;

typedef struct {
  CORBA_char magic[4];
  CORBA_char version[2];
  CORBA_octet flags;
  CORBA_octet message_type;
  CORBA_unsigned_long message_size;
} GIOPMsgHeader;

typedef enum 
{
  GIOP_REQUEST,
  GIOP_REPLY,
  GIOP_CANCELREQUEST,
  GIOP_LOCATEREQUEST,
  GIOP_LOCATEREPLY,
  GIOP_CLOSECONNECTION,
  GIOP_MESSAGEERROR,
  GIOP_FRAGMENT
} GIOPMsgType;

typedef enum
{
  GIOP_NO_EXCEPTION,
  GIOP_USER_EXCEPTION,
  GIOP_SYSTEM_EXCEPTION,
  GIOP_LOCATION_FORWARD
} GIOPReplyStatusType;

typedef enum
{
  GIOP_UNKNOWN_OBJECT,
  GIOP_OBJECT_HERE,
  GIOP_OBJECT_FORWARD
} GIOPLocateStatusType;

typedef struct {
  int dummy;
} IOP_ServiceContextList;

typedef struct {
} CORBA_Principal;

typedef struct {
} CORBA_sequence_IOP_TaggedComponent;

typedef gushort IOP_ProfileId;

typedef struct {
  IOP_ServiceContextList service_context;
  CORBA_unsigned_long request_id;
  CORBA_boolean response_expected;
  CORBA_sequence_octet object_key;
  CORBA_char *operation;
  CORBA_Principal requesting_principal;
} GIOPMsgRequest_1_0;

typedef struct {
  IOP_ServiceContextList service_context;
  CORBA_unsigned_long request_id;
  CORBA_boolean response_expected;
  CORBA_char reserved[3];
  CORBA_sequence_octet object_key;
  CORBA_char *operation;
  CORBA_Principal requesting_principal;
} GIOPMsgRequest_1_1;

typedef enum {
  GIOP_KeyAddr = 0,
  GIOP_ProfileAddr = 1,
  GIOP_ReferenceAddr = 2
} GIOP_AddressingDisposition;

#define IOP_TAG_ORBIT_SPECIFIC 0xbadfaecal
typedef struct {
  CORBA_char *unix_sock_path;
  CORBA_unsigned_long ipv6_port;
} TAG_ORBIT_SPECIFIC_info;

#define IOP_TAG_INTERNET_IOP 0
typedef struct {
  CORBA_char *host;
  CORBA_unsigned_short port;
} TAG_INTERNET_IOP_info;

#define IOP_TAG_MULTIPLE_COMPONENTS 1
typedef struct {
} TAG_MULTIPLE_COMPONENTS_info;

#define IOP_TAG_IRDA_IOP 0x4f425400
typedef struct {
  CORBA_octet lsap_sel; /* If 0xFF, LSAP_ANY */
  CORBA_unsigned_long addr; /* Device address */
  CORBA_char *name; /* service name, similar to port# semantics. */
  CORBA_sequence_octet object_key;
  CORBA_sequence_IOP_TaggedComponent components;
} TAG_IRDA_IOP_info;

#define IOP_TAG_IPV6_IOP 0x4f425401
typedef struct {
  CORBA_char *host;
  CORBA_unsigned_short port;
  CORBA_sequence_octet object_key;
  CORBA_sequence_IOP_TaggedComponent components;
} TAG_IPV6_IOP_info;

#define IOP_TAG_UNIX_IOP 0x4f425402
typedef struct {
  CORBA_char *sock_path;
  CORBA_sequence_octet object_key;
} TAG_UNIX_IOP_info;

typedef enum {
  CORBA_Security_NoProtection = 1,
  CORBA_Security_Integrity = 2,
  CORBA_Security_Confidentiality = 4,
  CORBA_Security_DetectReplay = 8,
  CORBA_Security_DetectMisordering = 16,
  CORBA_Security_EstablishTrustInTarget = 32,
  CORBA_Security_EstablishTrustInClient = 64
} CORBA_Security_AssociationOptions;

#define TAG_SSL_SEC_TRANS 20

typedef struct {
  CORBA_Security_AssociationOptions target_supports, target_requires;
  CORBA_unsigned_short port;
} TAG_SSL_SEC_TRANS_info;

typedef struct {
  CORBA_Security_AssociationOptions target_supports, target_requires;
  CORBA_char *service;
} TAG_SSL_SEC_TRANS_EXT_info; /* A more generic version of TAG_SSL_SEC_TRANS */

typedef struct {
  CORBA_octet giop_major, giop_minor;
  IOP_ProfileId profile_type;
  gpointer profile_info;
} IOP_TaggedProfile;

typedef struct {
  CORBA_Object ior;
} GIOP_IORAddressingInfo;

typedef struct {
  GIOP_AddressingDisposition _d;
  union {
    /* case KeyAddr */
    CORBA_sequence_octet object_key;
    /* case ProfileAddr */
    IOP_TaggedProfile profile;
    /* case ReferenceAddr */
    GIOP_IORAddressingInfo ior;
  } _u;
} GIOP_TargetAddress;

typedef struct {
  CORBA_unsigned_long request_id;
  CORBA_octet response_flags;
  CORBA_octet reserved[3];
  GIOP_TargetAddress target;
  CORBA_char *operation;
  IOP_ServiceContextList service_context;
} GIOPMsgRequest_1_2;

typedef struct {
  IOP_ServiceContextList service_context;
  CORBA_unsigned_long request_id;
  GIOPReplyStatusType reply_status;
} GIOPMsgReply;

typedef struct {
  CORBA_unsigned_long request_id;
} GIOPMsgCancelRequest;

typedef struct {
  CORBA_unsigned_long request_id;
  CORBA_sequence_octet object_key;
} GIOPMsgLocateRequest;

typedef struct {
  CORBA_unsigned_long request_id;
  GIOPLocateStatusType locate_status;
} GIOPMsgLocateReply;

typedef struct {
  union {
    GIOPMsgRequest_1_0 request_1_0;
    GIOPMsgRequest_1_1 request_1_1;
    GIOPMsgRequest_1_2 request_1_2;
    GIOPMsgReply reply;
    GIOPMsgCancelRequest cancel_request;
    GIOPMsgLocateRequest locate_request;
    GIOPMsgLocateReply locate_reply;
  } u;
} GIOPMsg;

#endif
