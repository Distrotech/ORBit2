#ifndef GIOP_SEND_BUFFER_H
#define GIOP_SEND_BUFFER_H 1

#include <orbit/IIOP/giop-types.h>
#include <orbit/IIOP/giop-connection.h>

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
void giop_send_buffer_unuse(GIOPSendBuffer *buf);
void giop_send_buffer_append(GIOPSendBuffer *buf, const guchar *mem, gulong len);
guchar *giop_send_buffer_append_indirect(GIOPSendBuffer *buf, const guchar *mem, gulong len);
void giop_send_buffer_align(GIOPSendBuffer *buf, gulong boundary);

int giop_send_buffer_write(GIOPSendBuffer *buf, GIOPConnection *cnx);

#endif
