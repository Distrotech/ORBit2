#ifndef GIOP_MSG_BUFFER_H
#define GIOP_MSG_BUFFER_H 1

#include <IIOP/giop-types.h>

enum _GIOPMsgType
{
  GIOP_REQUEST,
  GIOP_REPLY,
  GIOP_CANCELREQUEST,
  GIOP_LOCATEREQUEST,
  GIOP_LOCATEREPLY,
  GIOP_CLOSECONNECTION,
  GIOP_MESSAGEERROR,
  GIOP_FRAGMENT
};

struct _GIOPMessageHeader
{
  CORBA_char magic[4];
  CORBA_char GIOP_version[2];
  CORBA_octet flags;
  
  /*
   * We should really use GIOPMsgType
   * but that enum winds up being an int...
   */
  CORBA_octet message_type;
  
  CORBA_unsigned_long message_size;
};

struct _GIOPMsgBuffer
{
  GIOPConnection *connection; 
				 
  GIOPMessageHeader msg_header;

  union 
  {
    GIOPMsgRequest request;
    GIOPMsgReply reply;
    GIOPMsgCancelRequest cancel_request;
    GIOPMsgLocateRequest locate_request;
    GIOPMsgLocateReply locate_reply;
  } 
  msg;
};

#define GIOP_MSG_BUFFER(x) ((GIOPMsgBuffer *)x)
#define GIOP_MSG_HEADER(x) (&(GIOP_MSG_BUFFER((x))->msg_header))
#define GIOP_MSG_SIZE(x) (GIOP_MSG_HEADER((x))->message_size)
#define GIOP_MSG_TYPE(x) (GIOP_MSG_HEADER((x))->message_type)
#define GIOP_MSG_REAL_SIZE(x) (GIOP_MSG_SIZE((x)) + sizeof (GIOPMessageHeader))
#define GIOP_MSG_CONNECTION(x) ((GIOP_MSG_BUFFER((x))->connection))
#define GIOP_MSG_REQUEST(x) (&(GIOP_MSG_BUFFER((x))->msg.request))
#define GIOP_MSG_REPLY(x) (&(GIOP_MSG_BUFFER((x))->msg.reply))
#define GIOP_MSG_CANCELREQUEST(x) (&(GIOP_MSG_BUFFER((x))->msg.cancel_request))
#define GIOP_MSG_LOCATEREQUEST(x) (&(GIOP_MSG_BUFFER((x))->msg.locate_request))
#define GIOP_MSG_LOCATEREPLY(x) (&(GIOP_MSG_BUFFER((x))->msg.locate_reply))

#define GIOP_FLAG_BIG_ENDIAN 0
#define GIOP_FLAG_LITTLE_ENDIAN 1

#if G_BYTE_ORDER == G_BIG_ENDIAN
#  define GIOP_FLAG_ENDIANNESS GIOP_FLAG_BIG_ENDIAN
#elif G_BYTE_ORDER == G_LITTLE_ENDIAN
#  define GIOP_FLAG_ENDIANNESS GIOP_FLAG_LITTLE_ENDIAN
#else
#  error "Unsupported endianness on this system."
#endif

CORBA_unsigned_long giop_get_request_id(void);

#endif /* GIOP_MSG_BUFFER_H */
	
	
