#include "config.h"

#include <string.h>

#include "giop-private.h"
#include <orbit/GIOP/giop-types.h>
#include <orbit/GIOP/giop-recv-buffer.h>

/* A list of GIOPMessageQueueEntrys */
O_MUTEX_DEFINE_STATIC(giop_queued_messages_lock);
static GList *giop_queued_messages;

/* A list of incoming requests */
static GList *incoming_recv_buffer_list;
O_MUTEX_DEFINE_STATIC(incoming_recv_buffer_list_lock);
#ifdef ORBIT_THREADED
O_CONDVAR_DEFINE_STATIC(incoming_recv_buffer_list_condvar);
#endif

static void giop_recv_buffer_handle_fragmented(GIOPRecvBuffer *buf,
					       GIOPConnection *cnx);
static void giop_recv_list_push(GIOPRecvBuffer *buf, GIOPConnection *cnx);

void
giop_recv_buffer_init(void)
{
  O_MUTEX_INIT(incoming_recv_buffer_list_lock);
#ifdef ORBIT_THREADED
#if 0
  O_MUTEX_INIT(incoming_recv_buffer_list_condvar_lock);
#endif
  pthread_cond_init(&incoming_recv_buffer_list_condvar, NULL);
#endif
  O_MUTEX_INIT(giop_queued_messages_lock);
}

static gboolean
giop_GIOP_TargetAddress_demarshal(GIOPRecvBuffer *buf, GIOP_TargetAddress *value)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);

  buf->cur = ALIGN_ADDRESS(buf->cur, 2);
  if((buf->cur + 2) > buf->end)
    return TRUE;
  if(do_bswap)
    buf->msg.u.request_1_2.target._d = GUINT16_SWAP_LE_BE(*(guint16 *)buf->cur);
  else
    buf->msg.u.request_1_2.target._d = *(guint16 *)buf->cur;
  buf->cur += 2;

  switch(buf->msg.u.request_1_2.target._d)
    {
    case GIOP_KeyAddr:
      buf->cur = ALIGN_ADDRESS(buf->cur, 4);
      if((buf->cur + 4) > buf->end)
	return TRUE;
      buf->msg.u.request_1_2.target._u.object_key._release = CORBA_FALSE;
      if(do_bswap)
	buf->msg.u.request_1_2.target._u.object_key._length = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
      else
	buf->msg.u.request_1_2.target._u.object_key._length = *((guint32 *)buf->cur);
      buf->cur += 4;
      if((buf->cur + buf->msg.u.request_1_2.target._u.object_key._length) > buf->end
	 || (buf->cur + buf->msg.u.request_1_2.target._u.object_key._length) < buf->cur)
	return TRUE;
      buf->msg.u.request_1_2.target._u.object_key._buffer = buf->cur;
      buf->cur += buf->msg.u.request_1_2.target._u.object_key._length;
      break;
    case GIOP_ProfileAddr:
      g_warning("XXX FIXME GIOP_ProfileAddr not handled");
      return TRUE;
      break;
    case GIOP_ReferenceAddr:
      g_warning("XXX FIXME GIOP_ReferenceAddr not handled");
      return TRUE;
      break;
    }

  return FALSE;
}

static gboolean
giop_IOP_ServiceContextList_demarshal(GIOPRecvBuffer *buf, IOP_ServiceContextList *value)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);
  int i;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);

  if((buf->cur + 4) > buf->end)
    return TRUE;

  value->_release = CORBA_TRUE;
  if(do_bswap)
    value->_length = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    value->_length = *((guint32 *)buf->cur);

  if((buf->cur + (value->_length*8)) > buf->end)
    return TRUE;

  buf->cur += 4;
  value->_buffer = g_new(IOP_ServiceContext, value->_length);
  for(i = 0; i < value->_length; i++)
    {
      buf->cur = ALIGN_ADDRESS(buf->cur, 4);
      if((buf->cur + 8) > buf->end)
	return TRUE;

      if(do_bswap)
	{
	  value->_buffer[i].context_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
	  buf->cur += 4;
	  value->_buffer[i].context_data._length = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
	}
      else
	{
	  value->_buffer[i].context_id = *((guint32 *)buf->cur);
	  buf->cur += 4;
	  value->_buffer[i].context_data._length = *((guint32 *)buf->cur);
	}
      buf->cur += 4;
      if((buf->cur + value->_buffer[i].context_data._length) > buf->end
	 || (buf->cur + value->_buffer[i].context_data._length) < buf->cur)
	return TRUE;
      value->_buffer[i].context_data._buffer = buf->cur;
      buf->cur += value->_buffer[i].context_data._length;
      value->_buffer[i].context_data._release = CORBA_FALSE;
    }

  return FALSE;
}

static void
giop_IOP_ServiceContextList_free (IOP_ServiceContextList *value)
{
	if (value)
		g_free (value->_buffer);
}

gboolean
giop_recv_buffer_demarshal_request_1_1(GIOPRecvBuffer *buf)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);
  CORBA_unsigned_long oplen;

  buf->msg.u.request_1_1.service_context._buffer = NULL;
  if(giop_IOP_ServiceContextList_demarshal(buf, &buf->msg.u.request_1_1.service_context))
    return TRUE;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);

  if((buf->cur + 12) > buf->end)
    return TRUE;

  if(do_bswap)
    buf->msg.u.request_1_1.request_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    buf->msg.u.request_1_1.request_id = *((guint32 *)buf->cur);
  buf->cur += 4;
  buf->msg.u.request_1_1.response_expected = *buf->cur;
  buf->cur += 4;
  if(do_bswap)
    buf->msg.u.request_1_1.object_key._length = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    buf->msg.u.request_1_1.object_key._length = *((guint32 *)buf->cur);
  buf->cur += 4;
  
  if((buf->cur + buf->msg.u.request_1_1.object_key._length) > buf->end
     || (buf->cur + buf->msg.u.request_1_1.object_key._length) < buf->cur)
    return TRUE;

  buf->msg.u.request_1_1.object_key._buffer = buf->cur;
  buf->msg.u.request_1_1.object_key._release = CORBA_FALSE;

  buf->cur += buf->msg.u.request_1_1.object_key._length;
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return TRUE;
  if(do_bswap)
    oplen = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    oplen = *((guint32 *)buf->cur);
  buf->cur += 4;

  if((buf->cur + oplen) > buf->end
     || (buf->cur + oplen) < buf->cur)
    return TRUE;

  buf->msg.u.request_1_1.operation = buf->cur;
  buf->cur += oplen;
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return TRUE;

  if(do_bswap)
    buf->msg.u.request_1_1.requesting_principal._length = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    buf->msg.u.request_1_1.requesting_principal._length = *((guint32 *)buf->cur);

  buf->cur += 4;
  if((buf->cur + buf->msg.u.request_1_1.requesting_principal._length) > buf->end
     || (buf->cur + buf->msg.u.request_1_1.requesting_principal._length) < buf->cur)
    return TRUE;

  buf->msg.u.request_1_1.requesting_principal._buffer = buf->cur;
  buf->msg.u.request_1_1.requesting_principal._release = CORBA_FALSE;
  buf->cur += buf->msg.u.request_1_1.requesting_principal._length;

  return FALSE;
}

gboolean
giop_recv_buffer_demarshal_request_1_2(GIOPRecvBuffer *buf)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);
  CORBA_unsigned_long oplen;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);

  if((buf->cur + 8) > buf->end)
    return TRUE;

  if(do_bswap)
    buf->msg.u.request_1_2.request_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    buf->msg.u.request_1_2.request_id = *((guint32 *)buf->cur);
  buf->cur += 4;
  buf->msg.u.request_1_2.response_flags = *buf->cur;
  buf->cur += 4;

  if(giop_GIOP_TargetAddress_demarshal(buf, &buf->msg.u.request_1_2.target))
    return TRUE;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return TRUE;

  if(do_bswap)
    oplen = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    oplen = *((guint32 *)buf->cur);
  buf->cur += 4;

  if((buf->cur + oplen) > buf->end
     || (buf->cur + oplen) < buf->cur)
    return TRUE;

  buf->msg.u.request_1_2.operation = buf->cur;
  buf->cur += oplen;

  buf->msg.u.request_1_2.service_context._buffer = NULL;
  if(giop_IOP_ServiceContextList_demarshal(buf, &buf->msg.u.request_1_2.service_context))
    return TRUE;
  buf->cur = ALIGN_ADDRESS(buf->cur, 8);

  return FALSE;
}

gboolean
giop_recv_buffer_demarshal_reply_1_1(GIOPRecvBuffer *buf)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);

  buf->msg.u.reply_1_1.service_context._buffer = NULL;
  if(giop_IOP_ServiceContextList_demarshal(buf, &buf->msg.u.reply_1_1.service_context))
    return TRUE;
  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 8) > buf->end)
    return TRUE;
  if(do_bswap)
    {
      buf->msg.u.reply_1_1.request_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
      buf->cur += 4;
      buf->msg.u.reply_1_1.reply_status = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
    }
  else
    {
      buf->msg.u.reply_1_1.request_id = *((guint32 *)buf->cur);
      buf->cur += 4;
      buf->msg.u.reply_1_1.reply_status = *((guint32 *)buf->cur);
    }
  buf->cur += 4;

 return FALSE;
}

gboolean
giop_recv_buffer_demarshal_reply_1_2(GIOPRecvBuffer *buf)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 8) > buf->end)
    return TRUE;
  if(do_bswap)
    {
      buf->msg.u.reply_1_2.request_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
      buf->cur += 4;
      buf->msg.u.reply_1_2.reply_status = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
    }
  else
    {
      buf->msg.u.reply_1_2.request_id = *((guint32 *)buf->cur);
      buf->cur += 4;
      buf->msg.u.reply_1_2.reply_status = *((guint32 *)buf->cur);
    }
  buf->cur += 4;

  buf->msg.u.reply_1_2.service_context._buffer = NULL;
  if(giop_IOP_ServiceContextList_demarshal(buf, &buf->msg.u.reply_1_2.service_context))
    return TRUE;

  buf->cur = ALIGN_ADDRESS(buf->cur, 8);
  return FALSE;
}

gboolean
giop_recv_buffer_demarshal_cancel(GIOPRecvBuffer *buf)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return TRUE;
  if(do_bswap)
    buf->msg.u.cancel_request.request_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    buf->msg.u.cancel_request.request_id = *((guint32 *)buf->cur);
  buf->cur += 4;

  return FALSE;
}

gboolean
giop_recv_buffer_demarshal_locate_request_1_1(GIOPRecvBuffer *buf)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);

  if((buf->cur + 8) > buf->end)
    return TRUE;

  if(do_bswap)
    buf->msg.u.locate_request_1_1.request_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    buf->msg.u.locate_request_1_1.request_id = *((guint32 *)buf->cur);
  buf->cur += 4;
  if(do_bswap)
    buf->msg.u.locate_request_1_1.object_key._length = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    buf->msg.u.locate_request_1_1.object_key._length = *((guint32 *)buf->cur);
  buf->cur += 4;

  if((buf->cur + buf->msg.u.locate_request_1_1.object_key._length) > buf->end
     || (buf->cur + buf->msg.u.locate_request_1_1.object_key._length) < buf->cur)
    return TRUE;

  buf->msg.u.locate_request_1_1.object_key._buffer = buf->cur;
  buf->msg.u.locate_request_1_1.object_key._release = CORBA_FALSE;
  buf->cur += buf->msg.u.locate_request_1_1.object_key._length;

  return FALSE;
}

gboolean
giop_recv_buffer_demarshal_locate_request_1_2(GIOPRecvBuffer *buf)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);

  if((buf->cur + 4) > buf->end)
    return TRUE;

  if(do_bswap)
    buf->msg.u.locate_request_1_1.request_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
  else
    buf->msg.u.locate_request_1_1.request_id = *((guint32 *)buf->cur);
  buf->cur += 4;
  
  return giop_GIOP_TargetAddress_demarshal(buf, &buf->msg.u.request_1_2.target);
}

gboolean
giop_recv_buffer_demarshal_locate_reply_1_1(GIOPRecvBuffer *buf)
{
  gboolean do_bswap = giop_msg_conversion_needed(buf);

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);

  if((buf->cur + 8) > buf->end)
    return TRUE;

  if(do_bswap)
    {
      buf->msg.u.locate_reply_1_1.request_id = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
      buf->cur += 4;
      buf->msg.u.locate_reply_1_1.locate_status = GUINT32_SWAP_LE_BE(*((guint32 *)buf->cur));
    }
  else
    {
      buf->msg.u.locate_reply_1_1.request_id = *((guint32 *)buf->cur);
      buf->cur += 4;
      buf->msg.u.locate_reply_1_1.locate_status = *((guint32 *)buf->cur);
    }
  buf->cur += 4;
  
  return FALSE;
}

gboolean
giop_recv_buffer_demarshal_locate_reply_1_2(GIOPRecvBuffer *buf)
{
  return giop_recv_buffer_demarshal_locate_reply_1_1(buf);
}

typedef gboolean (*GIOPDecodeFunc)(GIOPRecvBuffer *buf);

gboolean
giop_recv_buffer_demarshal(GIOPRecvBuffer *buf)
{
  static const GIOPDecodeFunc decode_funcs[GIOP_NUM_MSG_TYPES][GIOP_NUM_VERSIONS] = {
    /* request */
    {giop_recv_buffer_demarshal_request_1_1, giop_recv_buffer_demarshal_request_1_1, giop_recv_buffer_demarshal_request_1_2},
    /* reply */
    {giop_recv_buffer_demarshal_reply_1_1, giop_recv_buffer_demarshal_reply_1_1, giop_recv_buffer_demarshal_reply_1_2},
    /* cancel request */
    {giop_recv_buffer_demarshal_cancel, giop_recv_buffer_demarshal_cancel, giop_recv_buffer_demarshal_cancel},
    /* locate request */
    {giop_recv_buffer_demarshal_locate_request_1_1, giop_recv_buffer_demarshal_locate_request_1_1,
     giop_recv_buffer_demarshal_locate_request_1_2},
    /* locate reply */
    {giop_recv_buffer_demarshal_locate_reply_1_1, giop_recv_buffer_demarshal_locate_reply_1_1,
     giop_recv_buffer_demarshal_locate_reply_1_2},
    /* message error */
    {NULL, NULL, NULL},
    /* fragment */
    {NULL, NULL, NULL}
  };
  GIOPDecodeFunc decode_func;

  if(buf->msg.header.message_type >= GIOP_NUM_MSG_TYPES)
    return TRUE;

  decode_func = decode_funcs[buf->msg.header.message_type][buf->giop_version];

  if(decode_func)
    return (* decode_func)(buf);

  return FALSE;
}

GIOPMessageInfo
giop_recv_buffer_state_change(GIOPRecvBuffer *buf, GIOPMessageBufferState state, gboolean is_auth, GIOPConnection *cnx)
{
  GIOPMessageInfo retval = GIOP_MSG_UNDERWAY;

  buf->state = state;

  switch(state)
    {
    case GIOP_MSG_READING_HEADER:
      buf->cur = (guchar *)&buf->msg.header;
      buf->left_to_read = 12;
      break;
    case GIOP_MSG_READING_BODY:
      /* Check the header */
      if(memcmp(buf->msg.header.magic, "GIOP", 4))
	goto msg_error;
      if(buf->msg.header.message_type
	 >= GIOP_NUM_MSG_TYPES)
	goto msg_error;
      switch(buf->msg.header.version[0])
	{
	case 1:
	  switch(buf->msg.header.version[1])
	    {
	    case 0:
	      buf->giop_version = GIOP_1_0;
	      break;
	    case 1:
	      buf->giop_version = GIOP_1_1;
	      break;
	    case 2:
	      buf->giop_version = GIOP_1_2;
	      break;
	    default:
	      goto msg_error;
	      break;
	    }
	  break;
	default:
	  goto msg_error;
	  break;
	}
      if((buf->msg.header.flags & GIOP_FLAG_LITTLE_ENDIAN) != GIOP_FLAG_ENDIANNESS)
	buf->msg.header.message_size = GUINT32_SWAP_LE_BE(buf->msg.header.message_size);
      if((buf->msg.header.message_size > GIOP_INITIAL_MSG_SIZE_LIMIT)
	 && !is_auth)
	goto msg_error;

      buf->message_body = g_malloc(buf->msg.header.message_size+12);
      buf->free_body = TRUE;
      buf->cur = buf->message_body + 12;
      buf->end = buf->cur + buf->msg.header.message_size;
      buf->left_to_read = buf->msg.header.message_size;
      break;
    case GIOP_MSG_READY:
      retval = GIOP_MSG_COMPLETE;
      buf->cur = buf->message_body + 12;
      if(giop_recv_buffer_demarshal(buf))
	goto msg_error;
      if(buf->msg.header.message_type == GIOP_FRAGMENT)
	giop_recv_buffer_handle_fragmented(buf, cnx);
      else
	giop_recv_list_push(buf, cnx);
      break;
    case GIOP_MSG_AWAITING_FRAGMENTS:
      retval = GIOP_MSG_COMPLETE;
      giop_recv_buffer_handle_fragmented(buf, cnx);
      break;
    }

  return retval;

 msg_error:
  buf->msg.header.message_type = GIOP_MESSAGEERROR;
  buf->msg.header.message_size = 0;
  return GIOP_MSG_INVALID;
}

GIOPRecvBuffer *
giop_recv_buffer_use_buf(gboolean is_auth)
{
  GIOPRecvBuffer *retval = NULL;

  retval = g_new0(GIOPRecvBuffer, 1);

  giop_recv_buffer_state_change(retval, GIOP_MSG_READING_HEADER, is_auth, NULL);

  return retval;
}

GIOPRecvBuffer *
giop_recv_buffer_use_encaps(guchar *mem, gulong len)
{
  GIOPRecvBuffer *buf = giop_recv_buffer_use_buf(FALSE);

  buf->cur = buf->message_body = mem;
  buf->end = buf->cur + len;
  buf->msg.header.message_size = len;
  buf->msg.header.flags = *(buf->cur++);
  buf->giop_version = GIOP_LATEST;
  buf->left_to_read = 0;
  buf->state = GIOP_MSG_READY;
  buf->free_body = FALSE;

  return buf;
}

GIOPRecvBuffer *
giop_recv_buffer_use_encaps_buf(GIOPRecvBuffer *buf)
{
  CORBA_unsigned_long len;
  guchar *ptr;

  buf->cur = ALIGN_ADDRESS(buf->cur, 4);
  if((buf->cur + 4) > buf->end)
    return NULL;
  len = *(CORBA_unsigned_long *)buf->cur;
  if(giop_msg_conversion_needed(buf))
    len = GUINT32_SWAP_LE_BE(len);
  buf->cur += 4;
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    {
      G_BREAKPOINT();
      return NULL;
    }
  ptr = buf->cur;
  buf->cur += len;

  return giop_recv_buffer_use_encaps(ptr, len);
}

void
giop_recv_buffer_unuse(GIOPRecvBuffer *buf)
{
	if (!buf)
		return;

	if (buf->free_body) {
		g_free(buf->message_body);
		buf->message_body = NULL;
	}

	switch (buf->giop_version) {
	case GIOP_1_0:
		break;
	case GIOP_1_1:
		switch(buf->msg.header.message_type) {
		case GIOP_REPLY:
			giop_IOP_ServiceContextList_free (&buf->msg.u.reply_1_1.service_context);
			break;
		case GIOP_REQUEST:
			giop_IOP_ServiceContextList_free (&buf->msg.u.request_1_1.service_context);
			break;
		default:
			break;
		}
		break;
	case GIOP_1_2:
		switch(buf->msg.header.message_type) {
		case GIOP_REPLY:
			giop_IOP_ServiceContextList_free (&buf->msg.u.reply_1_2.service_context);
			break;
		case GIOP_REQUEST:
			giop_IOP_ServiceContextList_free (&buf->msg.u.request_1_2.service_context);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}

	g_free (buf);
}

void
giop_recv_list_zap(GIOPConnection *cnx)
{
  GList *ltmp;
  GIOPMessageQueueEntry *ent;

  O_MUTEX_LOCK(giop_queued_messages_lock);
  for(ltmp = giop_queued_messages, ent = NULL; ltmp; ltmp = ltmp->next)
    {
      GIOPMessageQueueEntry *tmpent = ltmp->data;
      if(tmpent->cnx == cnx)
	{
	  ent = tmpent;
	  break;
	}
    }
  if(ent)
    {
      ent->buffer = NULL;
#ifdef ORBIT_THREADED
      pthread_cond_signal(&ent->condvar);
#endif
    }

  O_MUTEX_UNLOCK(giop_queued_messages_lock);
}

static void
giop_recv_list_push(GIOPRecvBuffer *buf, GIOPConnection *cnx)
{
  GList *ltmp;
  GIOPMessageQueueEntry *ent;

  buf->connection = cnx;
  switch(buf->msg.header.message_type)
    {
    case GIOP_REPLY:
    case GIOP_LOCATEREPLY:
      O_MUTEX_LOCK(giop_queued_messages_lock);
      for(ltmp = giop_queued_messages, ent = NULL; ltmp; ltmp = ltmp->next)
	{
	  GIOPMessageQueueEntry *tmpent = ltmp->data;
	  if(tmpent->msg_type == buf->msg.header.message_type
	     && tmpent->request_id == giop_recv_buffer_get_request_id(buf))
	    {
	      ent = tmpent;
	      break;
	    }
	}
      if(ent)
	{
	  ent->buffer = buf;
#ifdef ORBIT_THREADED
	  pthread_cond_signal(&ent->condvar);
#endif
	}
      else
	{
	  /*
	    the stub may have already sent the request but not gotten to
	    the giop_recv_buffer_use_reply() part of things yet.
	    Race condition probably best fixed by having the stub set up
	    waiting for a reply BEFORE sending the request.
	  */
	  giop_recv_buffer_unuse(buf);
	  g_error("This is a known bug that hasn't yet been fixed because the"
		  " most obvious solution involves creating other bugs. "
		  "Please make noise so this gets fixed.");
	}
      O_MUTEX_UNLOCK(giop_queued_messages_lock);
      break;
    default:
      O_MUTEX_LOCK(incoming_recv_buffer_list_lock);
      incoming_recv_buffer_list = g_list_prepend(incoming_recv_buffer_list,
						 buf);
      O_MUTEX_UNLOCK(incoming_recv_buffer_list_lock);
#ifdef ORBIT_THREADED
      pthread_cond_signal(&incoming_recv_buffer_list_condvar);
#endif
      break;
    }
}

CORBA_unsigned_long
giop_recv_buffer_get_request_id(GIOPRecvBuffer *buf)
{
  static const glong reqid_offsets[GIOP_NUM_MSG_TYPES][GIOP_NUM_VERSIONS] = {
    /* GIOP_REQUEST */
    { G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.request_1_0.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.request_1_1.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.request_1_2.request_id)},
    /* GIOP_REPLY */
    { G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.reply_1_0.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.reply_1_1.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.reply_1_2.request_id)},
    /* GIOP_CANCELREQUEST */
    { G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.cancel_request.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.cancel_request.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.cancel_request.request_id)},
    /* GIOP_LOCATEREQUEST */
    { G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.locate_request_1_0.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.locate_request_1_1.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.locate_request_1_2.request_id)},
    /* GIOP_LOCATEREPLY */
    { G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.locate_reply_1_0.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.locate_reply_1_1.request_id),
      G_STRUCT_OFFSET(GIOPRecvBuffer,
		      msg.u.locate_reply_1_2.request_id)},
    {0,0,0}, /* GIOP_MESSAGEERROR */
    {0,0,0} /* GIOP_FRAGMENT */
  };
  gulong offset;

  offset = reqid_offsets[buf->msg.header.message_type][buf->giop_version];
  if(!offset)
    return 0;

  return G_STRUCT_MEMBER(CORBA_unsigned_long, buf, offset);
}

void
giop_recv_list_destroy_queue_entry(GIOPMessageQueueEntry *ent)
{
  O_MUTEX_LOCK(giop_queued_messages_lock);
  giop_queued_messages = g_list_remove(giop_queued_messages, ent);
  O_MUTEX_UNLOCK(giop_queued_messages_lock);

#ifdef ORBIT_THREADED
  pthread_cond_destroy(&ent->condvar);
  O_MUTEX_DESTROY(ent->condvar_lock);
#endif
}

void
giop_recv_list_setup_queue_entry(GIOPMessageQueueEntry *ent,
				 GIOPConnection *cnx,
				 CORBA_unsigned_long msg_type,
				 CORBA_unsigned_long request_id)
{
  
#ifdef ORBIT_THREADED
  O_MUTEX_INIT(ent->condvar_lock);
  pthread_cond_init(&ent->condvar, NULL);
  O_MUTEX_LOCK(ent->condvar_lock);
#endif

  ent->cnx = cnx;
  ent->msg_type = msg_type;
  ent->request_id = request_id;

  O_MUTEX_LOCK(giop_queued_messages_lock);
  giop_queued_messages = g_list_prepend(giop_queued_messages, ent);
  O_MUTEX_UNLOCK(giop_queued_messages_lock);

  ent->buffer = NULL;
}

GIOPRecvBuffer *
giop_recv_buffer_get(GIOPMessageQueueEntry *ent,
		     gboolean block_for_reply)
{
#ifdef ORBIT_THREADED
  pthread_cond_wait(&ent->condvar, &ent->condvar_lock);
  O_MUTEX_UNLOCK(ent->condvar_lock);
#else
  g_main_iteration(block_for_reply);
  if(block_for_reply)
    {
      while(!ent->buffer && (ent->cnx->parent.status != LINC_DISCONNECTED))
	g_main_iteration(block_for_reply);
    }
#endif

  giop_recv_list_destroy_queue_entry(ent);

  return ent->buffer;
}

static GIOPRecvBuffer *
giop_recv_list_pop_T(void)
{
  GIOPRecvBuffer *retval = NULL;

  if(incoming_recv_buffer_list)
    {
      retval = incoming_recv_buffer_list->data;
      incoming_recv_buffer_list = g_list_remove(incoming_recv_buffer_list,
						retval);
    }

  return retval;
}

static GIOPRecvBuffer *
giop_recv_list_pop(void)
{
  GIOPRecvBuffer *retval;
  O_MUTEX_LOCK(incoming_recv_buffer_list_lock);
  retval = giop_recv_list_pop_T();
  O_MUTEX_UNLOCK(incoming_recv_buffer_list_lock);
  return retval;
}

GIOPRecvBuffer *
giop_recv_buffer_use(void)
{
  GIOPRecvBuffer *retval = NULL;

#ifdef ORBIT_THREADED
  O_MUTEX_LOCK(incoming_recv_buffer_list_lock);
  retval = giop_recv_list_pop_T();
  if(!retval)
    {
      pthread_cond_wait(&incoming_recv_buffer_list_condvar,
			&incoming_recv_buffer_list_lock);
      retval = giop_recv_list_pop_T();
    }
  O_MUTEX_UNLOCK(incoming_recv_buffer_list_lock);
#else
  while(!(retval = giop_recv_list_pop()))
    g_main_iteration(TRUE);
#endif

  return retval;
}

static void
giop_recv_buffer_handle_fragmented(GIOPRecvBuffer *buf, GIOPConnection *cnx)
{
  /* Drop fragmented packets on the floor for now */
  buf->connection = cnx;
  buf->end = buf->message_body + buf->msg.header.message_size;
  giop_recv_buffer_unuse(buf);
}

GIOPMessageInfo
giop_recv_buffer_data_read(GIOPRecvBuffer *buf, int n, gboolean is_auth,
			   GIOPConnection *cnx)
{
  GIOPMessageBufferState new_state;

  buf->left_to_read -= n;
  buf->cur += n;

  if(buf->left_to_read)
    return GIOP_MSG_UNDERWAY;

  switch(buf->state)
    {
    case GIOP_MSG_READING_HEADER:
      new_state = GIOP_MSG_READING_BODY;
      break;
    case GIOP_MSG_READING_BODY:
      if(buf->msg.header.flags & GIOP_FLAG_FRAGMENTED)
	new_state = GIOP_MSG_AWAITING_FRAGMENTS;
      else
	new_state = GIOP_MSG_READY;
      break;
    default:
      g_assert_not_reached();
      break;
    }

  return giop_recv_buffer_state_change(buf, new_state, is_auth, cnx);
}

guint
giop_recv_buffer_reply_status(GIOPRecvBuffer *buf)
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

CORBA_sequence_CORBA_octet *
giop_recv_buffer_get_objkey(GIOPRecvBuffer *buf)
{
  switch(buf->msg.header.version[1])
    {
    case 0:
      return &buf->msg.u.request_1_0.object_key;
      break;
    case 1:
      return &buf->msg.u.request_1_1.object_key;
      break;
    case 2:
      g_assert(buf->msg.u.request_1_2.target._d == GIOP_KeyAddr);
      return &buf->msg.u.request_1_2.target._u.object_key;
      break;
    }

  return NULL;
}

char *
giop_recv_buffer_get_opname(GIOPRecvBuffer *buf)
{
  switch(buf->msg.header.version[1])
    {
    case 0:
      return buf->msg.u.request_1_0.operation;
      break;
    case 1:
      return buf->msg.u.request_1_1.operation;
      break;
    case 2:
      return buf->msg.u.request_1_2.operation;
      break;
    }

  return NULL;
}
