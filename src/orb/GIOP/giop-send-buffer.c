#include "config.h"

#include <string.h>

#include "giop-private.h"
#include <orbit/GIOP/giop.h>
#include <sys/uio.h>

#define GIOP_CHUNK_ALIGN 8
#define GIOP_CHUNK_SIZE (GIOP_CHUNK_ALIGN*256)

static GSList *send_buffer_list;
O_MUTEX_DEFINE_STATIC(send_buffer_list_lock);
O_MUTEX_DEFINE_STATIC(request_id_lock);
static const char giop_zero_buf[GIOP_CHUNK_ALIGN] = {0};

static void giop_send_buffer_append_real(GIOPSendBuffer *buf, gconstpointer mem, gulong len);

void
giop_send_buffer_init(void)
{
  O_MUTEX_INIT(send_buffer_list_lock);
  O_MUTEX_INIT(request_id_lock);
}

CORBA_unsigned_long
giop_get_request_id(void)
{
  CORBA_unsigned_long retval;
  static CORBA_unsigned_long ctr = 0;

  O_MUTEX_LOCK(request_id_lock);
  retval = ctr++;
  O_MUTEX_UNLOCK(request_id_lock);

  return retval;
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
  giop_send_buffer_append_real(retval, (guchar *)&retval->msg.header, 12);
  retval->msg.header.message_size = 0;
  retval->header_size = 12;

  return retval;
}

#if 0
static void
giop_IOP_ServiceContextList_marshal(GIOPSendBuffer *buf, const IOP_ServiceContextList *service_context)
{
  guint i, n;

  n = service_context->_length;

  giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
  giop_send_buffer_append(buf, &service_context->_length, sizeof(service_context->_length));

  for(i = 0; i < n; i++)
    {
      giop_send_buffer_align(buf, sizeof(IOP_ServiceId));
      giop_send_buffer_append(buf, &service_context->_buffer[i].context_id, sizeof(IOP_ServiceId));
    }
}

static void
giop_CORBA_sequence_octet_marshal(GIOPSendBuffer *buf, const CORBA_sequence_octet *seq)
{
  giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
  giop_send_buffer_append(buf, &seq->_length, sizeof(CORBA_unsigned_long));
  giop_send_buffer_append(buf, seq->_buffer,
			  seq->_length);
}
#endif

/* Marshal it at compile time so we don't have to do it over and over. This just stores codeset info to say that
     we only speak UTF-8/UTF-16 */
static const CORBA_unsigned_long iop_service_context_data[] = {
  1 /* num_contexts */,
  1 /* ServiceId for CodeSets */,
  12 /* length of encapsulation: 4 endianness+align, 4 charset_id, 4 wcharset_id */,
#if G_BYTE_ORDER == G_LITTLE_ENDIAN
  0x01010101 /* start of encapsulation */,
#else
  0,
#endif
  0x05010001, /* UTF-8 */
  0x00010109 /* UTF-16 */
};

static const GIOP_AddressingDisposition giop_1_2_target_type = GIOP_KeyAddr;

GIOPSendBuffer *
giop_send_buffer_use_request(GIOPVersion giop_version,
			     CORBA_unsigned_long request_id,
			     CORBA_boolean response_expected,
			     const struct iovec *object_key_vec,
			     const struct iovec *operation_vec,
			     const struct iovec *principal_vec)
{
  GIOPSendBuffer *buf = giop_send_buffer_use(giop_version);

  buf->msg.header.message_type = GIOP_REQUEST;
  giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
  buf->msg.u.request_1_0.request_id = request_id;
  buf->msg.u.request_1_0.response_expected = response_expected;

  switch(giop_version)
    {
    case GIOP_1_0:
    case GIOP_1_1:
      giop_send_buffer_append(buf, (const guchar *)iop_service_context_data, sizeof(iop_service_context_data));
      giop_send_buffer_append(buf, &buf->msg.u.request_1_0.request_id, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, &buf->msg.u.request_1_0.response_expected, sizeof(CORBA_boolean));
      giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, object_key_vec->iov_base, object_key_vec->iov_len);
      giop_send_buffer_append(buf, operation_vec->iov_base, operation_vec->iov_len);
      giop_send_buffer_append(buf, principal_vec->iov_base, principal_vec->iov_len);
      break;
    case GIOP_1_2:
      giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, &buf->msg.u.request_1_0.request_id, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, &buf->msg.u.request_1_0.response_expected, sizeof(CORBA_boolean));
      giop_send_buffer_append(buf, giop_zero_buf, 3);
      giop_send_buffer_append(buf, &giop_1_2_target_type, 2); /* We always use GIOP::KeyAddr addressing - the only sane way */
      giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, object_key_vec->iov_base, object_key_vec->iov_len);
      g_assert(!(object_key_vec->iov_len % 4));
      giop_send_buffer_append(buf, operation_vec->iov_base, operation_vec->iov_len);
      giop_send_buffer_append(buf, (const guchar *)iop_service_context_data, sizeof(iop_service_context_data));
      giop_send_buffer_align(buf, 8); /* alignment for the body */
    default:
      break;
    }

  return buf;
}

GIOPSendBuffer *
giop_send_buffer_use_reply(GIOPVersion giop_version,
			   CORBA_unsigned_long request_id,
			   CORBA_unsigned_long reply_status)
{
  GIOPSendBuffer *buf = giop_send_buffer_use(giop_version);

  buf->msg.header.message_type = GIOP_REPLY;

  switch(giop_version)
    {
    case GIOP_1_0:
    case GIOP_1_1:
      buf->msg.u.reply_1_0.reply_status = reply_status;
      buf->msg.u.reply_1_0.request_id = request_id;
      giop_send_buffer_append(buf, (const guchar *)iop_service_context_data, sizeof(iop_service_context_data));      
      giop_send_buffer_append(buf, &buf->msg.u.reply_1_0.request_id, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, &buf->msg.u.reply_1_0.reply_status, sizeof(CORBA_unsigned_long));
      break;
    case GIOP_1_2:
      buf->msg.u.reply_1_2.reply_status = reply_status;
      buf->msg.u.reply_1_2.request_id = request_id;
      giop_send_buffer_append(buf, &buf->msg.u.reply_1_2.request_id, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, &buf->msg.u.reply_1_2.reply_status, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, (const guchar *)iop_service_context_data, sizeof(iop_service_context_data));
      giop_send_buffer_align(buf, 8); /* alignment for the body */
    default:
      break;
    }

  return buf;
}

#if 0
/* We never send this */
GIOPSendBuffer *
giop_send_buffer_use_cancel_request(void);
#endif

GIOPSendBuffer *
giop_send_buffer_use_locate_request(GIOPVersion giop_version,
				    CORBA_unsigned_long request_id,
				    const struct iovec *object_key_vec)
{
  GIOPSendBuffer *buf = giop_send_buffer_use(giop_version);

  buf->msg.header.message_type = GIOP_LOCATEREQUEST;

  buf->msg.u.locate_request_1_0.request_id = request_id;
  giop_send_buffer_append(buf, &buf->msg.u.locate_request_1_0.request_id, sizeof(CORBA_unsigned_long));

  switch(giop_version)
    {
    case GIOP_1_0:
    case GIOP_1_1:
      giop_send_buffer_append(buf, object_key_vec->iov_base, object_key_vec->iov_len);
      break;
    case GIOP_1_2:
      giop_send_buffer_append(buf, &giop_1_2_target_type, sizeof(giop_1_2_target_type));
      giop_send_buffer_align(buf, sizeof(CORBA_unsigned_long));
      giop_send_buffer_append(buf, object_key_vec->iov_base, object_key_vec->iov_len);
    default:
      break;
    }

  return buf;
}

GIOPSendBuffer *
giop_send_buffer_use_locate_reply(GIOPVersion giop_version,
				  CORBA_unsigned_long request_id,
				  CORBA_unsigned_long locate_status)
{
  GIOPSendBuffer *buf = giop_send_buffer_use(giop_version);

  buf->msg.header.message_type = GIOP_LOCATEREPLY;

  buf->msg.u.locate_reply_1_0.request_id = request_id;
  giop_send_buffer_append(buf, &buf->msg.u.locate_reply_1_0.request_id, sizeof(CORBA_unsigned_long));
  buf->msg.u.locate_reply_1_0.locate_status = locate_status;
  giop_send_buffer_append(buf, &buf->msg.u.locate_reply_1_0.locate_status, sizeof(CORBA_unsigned_long));

  return buf;
}

GIOPSendBuffer *
giop_send_buffer_use_close_connection(GIOPVersion giop_version)
{
  GIOPSendBuffer *buf = giop_send_buffer_use(giop_version);

  buf->msg.header.message_type = GIOP_CLOSECONNECTION;  
  
  return buf;
}

GIOPSendBuffer *
giop_send_buffer_use_message_error(GIOPVersion giop_version)
{
  GIOPSendBuffer *buf = giop_send_buffer_use(giop_version);

  buf->msg.header.message_type = GIOP_MESSAGEERROR;  
  
  return buf;
}

void
giop_send_buffer_unuse(GIOPSendBuffer *buf)
{
  O_MUTEX_LOCK(send_buffer_list_lock);
  send_buffer_list = g_slist_prepend(send_buffer_list, buf);
  O_MUTEX_UNLOCK(send_buffer_list_lock);
}

static void
giop_send_buffer_append_real(GIOPSendBuffer *buf, gconstpointer mem, gulong len)
{
  register gulong num_used;
  register const guchar *lastptr;

  g_assert(mem);

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

  buf->lastptr = ((const guchar *)mem) + len;
}

void
giop_send_buffer_append(GIOPSendBuffer *buf, gconstpointer mem, gulong len)
{
  if(len <= 32)
    giop_send_buffer_append_indirect(buf, mem, len);
  else
    giop_send_buffer_append_real(buf, mem, len);
}

guchar *
giop_send_buffer_append_indirect(GIOPSendBuffer *buf, gconstpointer mem, gulong len)
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
  giop_send_buffer_append_real(buf, indirect, len);

  buf->indirect = indirect + len;
  buf->indirect_left = indirect_left - len;

  return retval;
}

void
giop_send_buffer_align(GIOPSendBuffer *buf, gulong boundary)
{
  register gulong align_amt, ms;

  /* 1. Figure out how much to align by */
  ms = buf->msg.header.message_size + buf->header_size;
  align_amt = ALIGN_VALUE(ms, boundary) - ms;

  /* 2. Do the alignment */
  if(align_amt)
    {
      if(buf->lastptr == buf->indirect)
	giop_send_buffer_append_indirect(buf, NULL, align_amt);
      else
	giop_send_buffer_append(buf, giop_zero_buf, align_amt);
    }
}

int
giop_send_buffer_write(GIOPSendBuffer *buf, GIOPConnection *cnx)
{
  int retval;
  if(buf->giop_version >= GIOP_1_2)
    /* Do tail align */
    giop_send_buffer_align(buf, 8);

  O_MUTEX_LOCK(cnx->outgoing_mutex);

  retval = linc_connection_writev((LINCConnection *)cnx, buf->iovecs, buf->num_used, buf->msg.header.message_size + buf->header_size);

  O_MUTEX_UNLOCK(cnx->outgoing_mutex);

  return retval;
}
