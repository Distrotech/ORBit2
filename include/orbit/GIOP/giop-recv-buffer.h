#ifndef GIOP_RECV_BUFFER_H
#define GIOP_RECV_BUFFER_H 1

typedef enum {
  GIOP_MSG_READING_HEADER,
  GIOP_MSG_READING_BODY,
  GIOP_MSG_AWAITING_FRAGMENTS,
  GIOP_MSG_READY
} GIOPMessageBufferState;

struct _GIOPRecvBuffer {
  GIOPMsg msg;
  guchar *message_body;
  guchar *cur;

  gint left_to_read;
};

#endif
