#ifndef GIOP_RECV_BUFFER_H
#define GIOP_RECV_BUFFER_H 1

#include <IIOP/giop-types.h>

#define GIOP_RECV_BUFFER(x) ((GIOPRecvBuffer *)x)

enum _GIOPRecvBufferState{
  GIOP_RECV_BUFFER_READING_HEADER,
  GIOP_RECV_BUFFER_READING_BODY,
  GIOP_RECV_BUFFER_READY,
  GIOP_RECV_BUFFER_OVERSTEPPED
};

struct _GIOPRecvBuffer
{
  GIOPMsgBuffer msg_buffer;

  gpointer message;
  gpointer cur;

  GIOPRecvBufferState state;
  gint left_to_read;
};

void giop_recv_buffer_free(GIOPRecvBuffer *buffer);

GIOPRecvBuffer *giop_recv_buffer_read(GIOPConnection *connection);

#define GIOP_RECV_BUFFER_BODY(x) (PTR_ADD((x)->message, 		\
					  sizeof (GIOPMessageHeader)))

#define giop_recv_buffer_reply_request_id(recv_buffer) 			\
  ((GIOP_MSG_TYPE((recv_buffer)) == GIOP_REPLY) ? 			\
    GIOP_MSG_REPLY((recv_buffer))->request_id :				\
   (GIOP_MSG_TYPE((recv_buffer)) == GIOP_LOCATEREPLY) ?			\
    GIOP_MSG_LOCATEREPLY((recv_buffer))->request_id : 0)

#define giop_recv_buffer_conversion_needed(buffer) 			\
  ((GIOP_MSG_HEADER ((buffer))->flags & 1) != GIOP_FLAG_ENDIANNESS)


#define giop_recv_buffer_check_pos(buffer, pos)				\
G_STMT_START{								\
  if (PTR_CMP((pos), >, PTR_ADD( GIOP_RECV_BUFFER_BODY((buffer)), 	\
                                  GIOP_MSG_SIZE ((buffer))))         ||	\
      PTR_CMP((pos), <=, GIOP_RECV_BUFFER_BODY((buffer))))		\
    (buffer)->state = GIOP_RECV_BUFFER_OVERSTEPPED;			\
}G_STMT_END	

#define giop_recv_buffer_overstepped(buffer) 				\
  ((buffer)->state == GIOP_RECV_BUFFER_OVERSTEPPED)

#define giop_recv_buffer_get_atom(buffer, object)			\
G_STMT_START{								\
  gchar *obj_pos = ALIGN_ADDRESS ((buffer)->cur, sizeof ((object)));	\
  giop_recv_buffer_check_pos((buffer), obj_pos + sizeof ((object)));	\
  if (!giop_recv_buffer_overstepped((buffer)))				\
    {									\
      if (giop_recv_buffer_conversion_needed ((buffer))) 		\
        CONVERT_ENDIANNESS (&(object), obj_pos, sizeof ((object))); 	\
      else								\
        memcpy(&(object), obj_pos, sizeof((object)));			\
      (buffer)->cur = obj_pos + sizeof (object);			\
    }									\
  else									\
    memset(&(object), 0, sizeof ((object)));				\
}G_STMT_END

#define giop_recv_buffer_get_string(buffer, dest, len)			\
G_STMT_START{								\
  gchar *new_pos;							\
  giop_recv_buffer_get_atom((buffer), (len));				\
  new_pos = PTR_ADD ((buffer)->cur, len);				\
  giop_recv_buffer_check_pos((buffer), new_pos);			\
  if (!giop_recv_buffer_overstepped((buffer)))				\
    {									\
      (dest) = (buffer)->cur;						\
      (buffer)->cur = new_pos;						\
    }									\
  else									\
    (dest) = NULL;							\
}G_STMT_END

#define giop_recv_buffer_get_sequence_octet(buffer, sequence)		\
  giop_recv_buffer_get_string((buffer),(sequence)._buffer,(sequence)._length)

#endif /* GIOP_RECV_BUFFER_H */




