#ifndef GIOP_SEND_BUFFER_H
#define GIOP_SEND_BUFFER_H 1

#include <IIOP/giop-types.h>

/* The size of the chunks that are used for indirect pieces of
 * messages.  Too low, and you'll have a lot of malloc overhead. Too
 * high, and you'll get wasted mem. Their size *must* be a multiple of
 * the biggest alignment of all CORBA types. 8 should be a safe guess
 * for that. 
 *
 * NB: Changing this value will break binary compatibilty of ORBit. 
 *
 *         DO NOT CHANGE IT, UNLESS YOU KNOW WHAT YOU'RE DOING.
 */
#define GIOP_INDIRECT_CHUNK_SIZE (8*256)

#define GIOP_SEND_BUFFER(x) ((GIOPSendBuffer *)x)

struct _GIOPSendBuffer
{
  GIOPMsgBuffer msg_buffer;

  GIOPIovecArray *iovecs;

  gpointer indirect;
  GMemChunk *indirects;

  /* Writing position in indirect */
  gulong indirect_cur;

  /* TRUE, if current content of
   * [indirect+indirect_start, indirect+indirect_cur) not yet appended
   * to the iovecs to write */
  gboolean indirect_open;

  /* Start of the last unwritten memory region in indirect */
  gulong indirect_start;
};

gpointer
giop_send_buffer_copy_mem_cross_border (GIOPSendBuffer * send_buffer,
					gconstpointer src, gulong len);

void
giop_send_buffer_align_cross_border (GIOPSendBuffer * send_buffer,
				     gulong align_for);

GIOPSendBuffer *
giop_send_buffer_new_request (GIOPConnection * connection,
			      const IOP_ServiceContextList *service_context,
			      CORBA_unsigned_long request_id,
			      CORBA_boolean response_expected,
			      CORBA_sequence_octet *object_key,
			      guchar *operation,
			      gulong operation_len,
			      CORBA_sequence_octet *principal);

GIOPSendBuffer *
giop_send_buffer_new_reply (GIOPConnection *connection,
			    const IOP_ServiceContextList *service_context,
			    CORBA_unsigned_long request_id,
			    GIOPReplyStatusType reply_status);

GIOPSendBuffer *
giop_send_buffer_new_locate_request (GIOPConnection *connection,
				     CORBA_unsigned_long request_id,
				     CORBA_sequence_octet *object_key);

GIOPSendBuffer *
giop_send_buffer_new_locate_reply (GIOPConnection *connection,
				   CORBA_unsigned_long request_id,
				   GIOPLocateStatusType reply_status);

gint giop_send_buffer_write (GIOPSendBuffer *request_buffer);

void giop_send_buffer_free (GIOPSendBuffer *send_buffer);

#define giop_send_buffer_align(buffer, align_for)			\
G_STMT_START{								\
  gulong real_msgsize = GIOP_MSG_REAL_SIZE ((buffer));			\
  gulong align_diff = (gulong)ALIGN_ADDRESS(real_msgsize, align_for) -	\
    real_msgsize;							\
  if ((align_for) + (buffer)->indirect_cur > GIOP_INDIRECT_CHUNK_SIZE)	\
    giop_send_buffer_align_cross_border ((buffer), (align_for));	\
  else if(align_diff > 0)						\
    {									\
      if ((buffer)->indirect_open)					\
        {								\
          (buffer)->indirect_cur += align_diff;				\
        } 								\
      else 								\
        {								\
          (buffer)->indirect_open = TRUE;				\
          (buffer)->indirect_cur = 					\
          (gulong)ALIGN_ADDRESS((buffer)->indirect_cur + align_diff,	\
			    align_for);					\
          (buffer)->indirect_start = buffer->indirect_cur - align_diff; \
        }								\
     GIOP_MSG_HEADER ((buffer))->message_size += align_diff;		\
  }									\
}G_STMT_END

#define giop_send_buffer_copy_mem(buffer, src, len)			\
G_STMT_START{								\
  if ((len) + (buffer)->indirect_cur > GIOP_INDIRECT_CHUNK_SIZE)	\
    giop_send_buffer_copy_mem_cross_border ((buffer), (src), (len));	\
  else									\
    {									\
      if (!(buffer)->indirect_open)					\
        {								\
          (buffer)->indirect_open = TRUE;				\
          (buffer)->indirect_start = (buffer)->indirect_cur;		\
        } 								\
      memcpy(((guchar*)(buffer)->indirect) + (buffer)->indirect_cur,	\
	     (src), (len));						\
      (buffer)->indirect_cur += (len);					\
      GIOP_MSG_HEADER ((buffer))->message_size += (len);  		\
    }									\
}G_STMT_END

#define giop_send_buffer_append_open_indirect(buffer)			\
G_STMT_START{								\
  if ((buffer)->indirect_open)						\
    {									\
      giop_iovec_array_append ((buffer)->iovecs, (buffer)->indirect +	\
	                       (buffer)->indirect_start, 		\
		               (buffer)->indirect_cur -  		\
			       (buffer)->indirect_start);		\
      (buffer)->indirect_open = FALSE;					\
    }									\
}G_STMT_END

#define giop_send_buffer_put_mem(buffer, src, len)			\
G_STMT_START{								\
  giop_send_buffer_append_open_indirect (buffer);			\
  giop_iovec_array_append ((buffer)->iovecs, (src), (len));		\
  GIOP_MSG_HEADER ((buffer))->message_size += (len);  	 		\
}G_STMT_END

#define giop_send_buffer_copy_atom(buffer, object)			\
G_STMT_START{								\
  giop_send_buffer_align ((buffer), sizeof ((object)));			\
  giop_send_buffer_copy_mem ((buffer), &(object), sizeof ((object)));	\
}G_STMT_END

#define giop_send_buffer_put_string(buffer, src, len)			\
G_STMT_START{								\
  CORBA_unsigned_long str_len = (len); 					\
  giop_send_buffer_copy_atom((buffer), str_len);			\
  if (str_len > 0)							\
    giop_send_buffer_put_mem ((buffer), (src), str_len);		\
}G_STMT_END

#define giop_send_buffer_copy_string(buffer, src, len)			\
G_STMT_START{								\
  CORBA_unsigned_long str_len = (len); 					\
  giop_send_buffer_copy_atom((buffer), str_len);			\
  if (str_len > 0)							\
    giop_send_buffer_copy_mem ((buffer), (src), str_len);		\
}G_STMT_END

#define giop_send_buffer_put_sequence_octet(buffer, sequence)		\
  giop_send_buffer_put_string((buffer),(sequence)._buffer,(sequence)._length)

#define giop_send_buffer_copy_sequence_octet(buffer, sequence)		\
  giop_send_buffer_copy_string((buffer),(sequence)._buffer,(sequence)._length)

#endif /* GIOP_SEND_BUFFER_H */
