#ifndef GIOP_RECV_BUFFER_H
#define GIOP_RECV_BUFFER_H 1

typedef enum {
  GIOP_MSG_READING_HEADER,
  GIOP_MSG_READING_BODY,
  GIOP_MSG_AWAITING_FRAGMENTS,
  GIOP_MSG_READY
} GIOPMessageBufferState;

typedef enum {
  GIOP_MSG_UNDERWAY,
  GIOP_MSG_COMPLETE,
  GIOP_MSG_INVALID
} GIOPMessageInfo;

struct _GIOPRecvBuffer {
  GIOPMsg msg;

  guchar *message_body;
  guchar *cur;

  GIOPMessageBufferState state;
  GIOPVersion giop_version;
  gulong left_to_read;
};

GIOPRecvBuffer *giop_recv_buffer_use_buf(gboolean is_auth);

GIOPRecvBuffer *giop_recv_buffer_use_reply(CORBA_unsigned_long request_id, gboolean block_for_reply);
GIOPRecvBuffer *giop_recv_buffer_use_locate_reply(CORBA_unsigned_long request_id, gboolean block_for_reply);
GIOPRecvBuffer *giop_recv_buffer_use(void);
void giop_recv_buffer_unuse(GIOPRecvBuffer *buf);
GIOPMessageInfo giop_recv_buffer_state_change(GIOPRecvBuffer *buf, GIOPMessageBufferState state, gboolean is_auth);
GIOPMessageInfo giop_recv_buffer_data_read(GIOPRecvBuffer *buf, int n, gboolean is_auth);

#endif
