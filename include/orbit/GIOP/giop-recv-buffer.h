#ifndef GIOP_RECV_BUFFER_H
#define GIOP_RECV_BUFFER_H 1

#include <orbit/GIOP/giop-types.h>

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
  guchar *end;
  GIOPConnection *connection;

  GIOPMessageBufferState state;
  GIOPVersion giop_version;
  gulong left_to_read;
  gboolean free_body : 1;
};

#define giop_msg_conversion_needed(msg) giop_endian_conversion_needed(GIOP_MSG(msg)->header.flags)
GIOPRecvBuffer *giop_recv_buffer_use_buf(gboolean is_auth);
GIOPRecvBuffer *giop_recv_buffer_use_encaps_buf(GIOPRecvBuffer *buf);
GIOPRecvBuffer *giop_recv_buffer_use_encaps(guchar *mem, gulong len);

GIOPRecvBuffer *giop_recv_buffer_use_reply(GIOPConnection *cnx, CORBA_unsigned_long request_id, gboolean block_for_reply);
GIOPRecvBuffer *giop_recv_buffer_use_locate_reply(GIOPConnection *cnx, CORBA_unsigned_long request_id, gboolean block_for_reply);
GIOPRecvBuffer *giop_recv_buffer_use(void);
void giop_recv_buffer_unuse(GIOPRecvBuffer *buf);
GIOPMessageInfo giop_recv_buffer_state_change(GIOPRecvBuffer *buf, GIOPMessageBufferState state, gboolean is_auth);
GIOPMessageInfo giop_recv_buffer_data_read(GIOPRecvBuffer *buf, int n, gboolean is_auth);
extern inline guint giop_recv_buffer_reply_status(GIOPRecvBuffer *buf)
{
  switch(buf->msg.header.version[1])
    {
    case 0:
      return buf->msg.u.reply_1_0.reply_status;
      break;
    case 1:
      return buf->msg.u.reply_1_1.reply_status;
      break;
    case 2:
      return buf->msg.u.reply_1_2.reply_status;
      break;
    }

  return 0;
}
CORBA_unsigned_long giop_recv_buffer_get_request_id(GIOPRecvBuffer *buf);
char *giop_recv_buffer_get_opname(GIOPRecvBuffer *buf);

#endif
