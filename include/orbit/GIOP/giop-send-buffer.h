#ifndef GIOP_SEND_BUFFER_H
#define GIOP_SEND_BUFFER_H 1

#include <orbit/GIOP/giop-types.h>
#include <orbit/GIOP/giop-connection.h>

typedef struct {
  gulong size;
  guchar *ptr;
} GIOPIndirectChunk;

struct _GIOPSendBuffer {
  GIOPMsg msg;

  struct iovec *iovecs;
  gulong num_alloced, num_used;

  const guchar *lastptr;

  guchar *indirect;
  gulong indirect_left;

  GIOPIndirectChunk *indirects;
  guint num_indirects_alloced, num_indirects_used;
  GIOPVersion giop_version;
};

GIOPSendBuffer *giop_send_buffer_use(GIOPVersion giop_version);

/* The object_key_vec and operation_vec must be tail aligned to sizeof(CORBA_unsigned_long) */
GIOPSendBuffer *giop_send_buffer_use_request(GIOPVersion giop_version,
					     CORBA_unsigned_long request_id,
					     CORBA_boolean response_expected,
					     const struct iovec *object_key_vec,
					     const struct iovec *operation_vec,
					     const struct iovec *principal_vec);
/* No cancel - we never send it */
GIOPSendBuffer *giop_send_buffer_use_reply(GIOPVersion giop_version,
					   CORBA_unsigned_long request_id,
					   CORBA_unsigned_long reply_status);
GIOPSendBuffer *giop_send_buffer_use_locate_request(GIOPVersion giop_version,
						    CORBA_unsigned_long request_id,
						    const struct iovec *object_key_vec);
GIOPSendBuffer *giop_send_buffer_use_locate_reply(GIOPVersion giop_version,
						  CORBA_unsigned_long request_id,
						  CORBA_unsigned_long locate_status);
GIOPSendBuffer *giop_send_buffer_use_close_connection(GIOPVersion giop_version);
GIOPSendBuffer *giop_send_buffer_use_message_error(GIOPVersion giop_version);

void giop_send_buffer_unuse(GIOPSendBuffer *buf);
void giop_send_buffer_append(GIOPSendBuffer *buf, gconstpointer mem, gulong len);
guchar *giop_send_buffer_append_indirect(GIOPSendBuffer *buf, gconstpointer mem, gulong len);
void giop_send_buffer_align(GIOPSendBuffer *buf, gulong boundary);

int giop_send_buffer_write(GIOPSendBuffer *buf, GIOPConnection *cnx);

#endif
