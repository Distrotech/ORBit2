#include "config.h"

#include <string.h>

#include "giop-private.h"
#include <orbit/GIOP/giop-types.h>
#include <orbit/GIOP/giop-send-buffer.h>
#include <sys/uio.h>

#define GIOP_CHUNK_ALIGN 8
#define GIOP_CHUNK_SIZE (GIOP_CHUNK_ALIGN*256)

static GSList *send_buffer_list;
O_MUTEX_DEFINE_STATIC(send_buffer_list_lock);

void
giop_send_buffer_init(void)
{
  O_MUTEX_INIT(send_buffer_list_lock);
}

GIOPSendBuffer *
giop_send_buffer_use(GIOPVersion giop_version)
{
  GIOPSendBuffer *retval;

  O_MUTEX_LOCK(send_buffer_list_lock);
  if(send_buffer_list)
    {
      GSList *ltmp;
      ltmp = send_buffer_list;
      send_buffer_list = g_slist_remove_link(send_buffer_list, ltmp);
      O_MUTEX_UNLOCK(send_buffer_list_lock);

      retval = ltmp->data;
      g_slist_free_1(ltmp);
      retval->num_used = retval->indirect_left = retval->num_indirects_used = 0;
    }
  else
    {
      O_MUTEX_UNLOCK(send_buffer_list_lock);

      retval = g_new0(GIOPSendBuffer, 1);

      memcpy(retval->msg.header.magic, "GIOP", 4);
      retval->msg.header.flags = GIOP_FLAG_ENDIANNESS;
      retval->num_alloced = 8;
      retval->iovecs = g_new(struct iovec, 8);
    }

  memcpy(retval->msg.header.version, giop_version_ids[giop_version], 2);
  retval->giop_version = giop_version;
  g_assert(sizeof(retval->msg.header) == 12);
  giop_send_buffer_append(retval, (guchar *)&retval->msg.header, 12);
  retval->msg.header.message_size = 0;

  return retval;
}

void
giop_send_buffer_unuse(GIOPSendBuffer *buf)
{
  O_MUTEX_LOCK(send_buffer_list_lock);
  send_buffer_list = g_slist_prepend(send_buffer_list, buf);
  O_MUTEX_UNLOCK(send_buffer_list_lock);
}

void
giop_send_buffer_append(GIOPSendBuffer *buf, const guchar *mem, gulong len)
{
  register gulong num_used;
  register const guchar *lastptr;

  lastptr = buf->lastptr;
  num_used = buf->num_used;
  if(num_used && mem == lastptr)
    {
      buf->iovecs[num_used-1].iov_len += len;
    }
  else
    {
      if(num_used >= buf->num_alloced)
	{
	  buf->num_alloced = MAX(buf->num_alloced, 4)*2;
	  buf->iovecs = g_realloc(buf->iovecs, buf->num_alloced * sizeof(struct iovec));
	}

      buf->iovecs[num_used].iov_base = (gpointer)mem;
      buf->iovecs[num_used].iov_len = len;
      buf->num_used = num_used + 1;
    }

  buf->msg.header.message_size += len;

  buf->lastptr = mem + len;
}

guchar *
giop_send_buffer_append_indirect(GIOPSendBuffer *buf, const guchar *mem, gulong len)
{
  register guchar *indirect;
  register gulong indirect_left;
  register guchar *retval;

  indirect = buf->indirect;
  indirect_left = buf->indirect_left;

  while(indirect_left < len)
    {
      register gulong num_indirects_used = buf->num_indirects_used;

      if(num_indirects_used >= buf->num_indirects_alloced)
	{
	  register gulong new_size;

	  buf->num_indirects_alloced++;
	  buf->indirects = g_realloc(buf->indirects, buf->num_indirects_alloced*sizeof(GIOPIndirectChunk));
	  new_size = (len + 7) & ~7;
	  if(GIOP_CHUNK_SIZE > new_size)
	    new_size = GIOP_CHUNK_SIZE;
	  buf->indirects[num_indirects_used].size = new_size;
	  buf->indirects[num_indirects_used].ptr = g_malloc(new_size);
	}

      indirect = buf->indirects[num_indirects_used].ptr;
      indirect_left = buf->indirects[num_indirects_used].size;
      buf->num_indirects_used = num_indirects_used + 1;
    }

  retval = indirect;
  if(mem)
    memcpy(indirect, mem, len);
  giop_send_buffer_append(buf, indirect, len);

  buf->indirect = indirect + len;
  buf->indirect_left = indirect_left - len;

  return retval;
}

void
giop_send_buffer_align(GIOPSendBuffer *buf, gulong boundary)
{
  register gulong align_amt, ms;
  static const char align_buf[GIOP_CHUNK_ALIGN];

  /* 1. Figure out how much to align by */
  ms = buf->msg.header.message_size + 12 /* size of message header */;
  align_amt = ALIGN_VALUE(ms, boundary);

  /* 2. Do the alignment */
  if(align_amt)
    {
      if(buf->lastptr == buf->indirect)
	giop_send_buffer_append_indirect(buf, NULL, align_amt);
      else
	giop_send_buffer_append(buf, align_buf, align_amt);
    }
}

int
giop_send_buffer_write(GIOPSendBuffer *buf, GIOPConnection *cnx)
{
  if(buf->giop_version >= GIOP_1_2)
    /* Do tail align */
    giop_send_buffer_align(buf, 8); 

  return linc_connection_writev((LINCConnection *)cnx, buf->iovecs, buf->num_used, buf->msg.header.message_size + 12);
}
