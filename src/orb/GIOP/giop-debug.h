#ifndef __GIOP_DEBUG__
#define __GIOP_DEBUG__

/*
 * Flip this switch to dump GIOP messages
 * as they are sent and received.
 */
#undef GIOP_DEBUG

#ifndef GIOP_DEBUG

#define do_giop_dump(fh, ptr, len, off)
#define do_giop_dump_send(buff)
#define do_giop_dump_recv(buff)

static inline void giop_dprintf (const char *format, ...) { };

#else /* GIOP_DEBUG */

#define do_giop_dump(fh, ptr, len, off)		giop_dump (fh, ptr, len, off)
#define do_giop_dump_send(buff)			giop_dump_send (buff)
#define do_giop_dump_recv(buff)			giop_dump_recv (buff)

#include <stdio.h>

#define giop_dprintf(format...) fprintf(stderr, format)

#endif /* GIOP_DEBUG */

void giop_dump      (FILE *out, guint8 const *ptr, guint32 len, guint32 offset);
void giop_dump_send (GIOPSendBuffer *send_buffer);
void giop_dump_recv (GIOPRecvBuffer *recv_buffer);

#endif /* __GIOP_DEBUG__ */
