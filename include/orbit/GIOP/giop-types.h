#ifndef GIOP_TYPES_H
#define GIOP_TYPES_H 1

#include <glib.h>
#include <glib-object.h>
#include <orbit/util/orbit-util.h>

typedef struct _GIOPRecvBuffer GIOPRecvBuffer;
typedef struct _GIOPSendBuffer GIOPSendBuffer;
typedef enum _GIOPReplyStatusType GIOPReplyStatusType;
typedef enum _GIOPLocateStatusType GIOPLocateStatusType;
typedef struct _GIOPMsgRequest GIOPMsgRequest;
typedef struct _GIOPMsgReply GIOPMsgReply;
typedef struct _GIOPMsgCancelRequest GIOPMsgCancelRequest;
typedef struct _GIOPMsgLocateRequest GIOPMsgLocateRequest;
typedef struct _GIOPMsgLocateReply GIOPMsgLocateReply;
typedef struct _GIOPMsg GIOPMsg;

typedef enum {
  GIOP_CONNECTION_SSL
} GIOPConnectionOptions;

#endif
