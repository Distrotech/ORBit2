#ifndef GIOP_MSG_H
#define GIOP_MSG_H 1

/* For IOP_ServiceContextList */
#include <orb/iop.h>

#include <IIOP/giop-types.h>

enum _GIOPReplyStatusType
{
  GIOP_NO_EXCEPTION,
  GIOP_USER_EXCEPTION,
  GIOP_SYSTEM_EXCEPTION,
  GIOP_LOCATION_FORWARD
};

enum _GIOPLocateStatusType
{
  GIOP_UNKNOWN_OBJECT,
  GIOP_OBJECT_HERE,
  GIOP_OBJECT_FORWARD
};


struct _GIOPMsgRequest
{
  IOP_ServiceContextList service_context;
  CORBA_unsigned_long request_id;
  CORBA_boolean response_expected;
  CORBA_sequence_octet object_key;
  CORBA_char *operation;
  CORBA_Principal requesting_principal;
};

struct _GIOPMsgReply
{
  IOP_ServiceContextList service_context;
  CORBA_unsigned_long request_id;
  GIOPReplyStatusType reply_status;
};

struct _GIOPMsgCancelRequest
{
  CORBA_unsigned_long request_id;
};

struct _GIOPMsgLocateRequest
{
  CORBA_unsigned_long request_id;
  CORBA_sequence_octet object_key;
};

struct _GIOPMsgLocateReply
{
  CORBA_unsigned_long request_id;
  GIOPLocateStatusType locate_status;
};

#endif /* GIOP_MSG_H */
