#ifndef GIOP_TYPES_H
#define GIOP_TYPES_H 1

#include <glib.h>

/* giop-msg.h */

typedef enum _GIOPReplyStatusType GIOPReplyStatusType;
typedef enum _GIOPLocateStatusType GIOPLocateStatusType;
typedef struct _GIOPMsgRequest GIOPMsgRequest;
typedef struct _GIOPMsgReply GIOPMsgReply;
typedef struct _GIOPMsgCancelRequest GIOPMsgCancelRequest;
typedef struct _GIOPMsgLocateRequest GIOPMsgLocateRequest;
typedef struct _GIOPMsgLocateReply GIOPMsgLocateReply;
typedef struct _GIOPMsg GIOPMsg;

/* giop-msg-buffer.h */

typedef enum _GIOPMsgType GIOPMsgType;
typedef struct _GIOPMessageHeader GIOPMessageHeader;
typedef struct _GIOPMsgBuffer GIOPMsgBuffer;

/* giop-recv-buffer.h */

typedef enum _GIOPRecvBufferState GIOPRecvBufferState;
typedef struct _GIOPRecvBuffer GIOPRecvBuffer;

/* giop-send-buffer.h */

typedef struct _GIOPSendBuffer GIOPSendBuffer;

/* giop-connection.h */

typedef enum _GIOPConnectionClass GIOPConnectionClass;
typedef struct _GIOPConnection GIOPConnection;
typedef void (*GIOPAddConnectionFunc) (GIOPConnection * connection);
typedef void (*GIOPRemoveConnectionFunc) (GIOPConnection * connection);
typedef void (*GIOPMainLoopIteration) (gboolean block_for_reply);
typedef void (*GIOPIncomingMessageFunc) (GIOPRecvBuffer * recv_buffer);
typedef GIOPRecvBuffer * (*GIOPWaitForReplyFunc) (GIOPConnection * connection,
						  GArray *request_ids,
						  gboolean block_for_reply);
typedef void (*GIOPNotifyReplyFunc) (GIOPRecvBuffer * recv_buffer, 
				     CORBA_unsigned_long request_id);
typedef GArray GIOPIovecArray;
						  
#endif GIOP_TYPES_H 1


