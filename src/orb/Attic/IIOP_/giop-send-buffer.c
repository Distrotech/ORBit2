#include "IIOP-private.h"

#include <string.h>

G_LOCK_DEFINE_STATIC (send_buffer_list);
static GSList *send_buffer_list = NULL;

gint giop_send_buffer_write (GIOPSendBuffer * send_buffer)
{
  if (!GIOP_MSG_CONNECTION (send_buffer)->is_valid)
    return -1;

  giop_send_buffer_append_open_indirect (send_buffer);

  return giop_connection_writev (GIOP_MSG_CONNECTION (send_buffer),
				 send_buffer->iovecs,
				 GIOP_MSG_REAL_SIZE (send_buffer)) ? 0 : -1;
}

static GIOPSendBuffer *
giop_send_buffer_new (GIOPConnection * connection)
{
  GIOPSendBuffer *send_buffer;

  if (!connection->is_valid)
    return NULL;

  G_LOCK (send_buffer_list);

  if (send_buffer_list)
    {
      GSList *head;

      send_buffer = send_buffer_list->data;

      head = send_buffer_list;
      /* FIXME: use g_slist_delete_link, once in GLib */
      send_buffer_list = g_slist_remove_link (send_buffer_list,
					      send_buffer_list);
      g_slist_free_1 (head);

      g_mem_chunk_reset (send_buffer->indirects);

      giop_iovec_array_reset (send_buffer->iovecs);  
    }
  else
    {
      send_buffer = g_new0 (GIOPSendBuffer, 1);;

      g_memmove (GIOP_MSG_HEADER (send_buffer)->magic, "GIOP", 4);
      GIOP_MSG_HEADER (send_buffer)->GIOP_version[0] = 1;
      GIOP_MSG_HEADER (send_buffer)->GIOP_version[1] = 0;
      GIOP_MSG_HEADER (send_buffer)->flags = GIOP_FLAG_ENDIANNESS;

      send_buffer->indirects =
	g_mem_chunk_create (char[GIOP_INDIRECT_CHUNK_SIZE], 2, G_ALLOC_ONLY);

      send_buffer->iovecs = giop_iovec_array_new ();
    }

  G_UNLOCK (send_buffer_list);

  giop_connection_ref (connection);
  GIOP_MSG_CONNECTION (send_buffer) = connection;
  GIOP_MSG_SIZE (send_buffer) = 0;

  giop_iovec_array_append (send_buffer->iovecs, GIOP_MSG_HEADER (send_buffer),
			   sizeof (GIOPMessageHeader));

  send_buffer->indirect = g_chunk_new (gpointer, send_buffer->indirects);
  send_buffer->indirect_cur = 0;
  send_buffer->indirect_open = FALSE;

  return send_buffer;
}

static void
giop_send_buffer_service_context_encode (GIOPSendBuffer * send_buffer,
					 const IOP_ServiceContextList *
					 context)
{
  static const CORBA_unsigned_long zero = 0;
  if (!context)
    {
      giop_send_buffer_copy_atom (send_buffer, zero);
    }
  else
    {
      int i;
      giop_send_buffer_copy_atom (send_buffer, context->_length);
      for (i = 0; i < context->_length; i++)
	{
	  giop_send_buffer_copy_atom (send_buffer,
				      context->_buffer[i].context_id);
	  giop_send_buffer_put_sequence_octet (send_buffer,
					       context->_buffer[i].
					       context_data);
	}
    }
}

GIOPSendBuffer *
giop_send_buffer_new_reply (GIOPConnection * connection,
			    const IOP_ServiceContextList * service_context,
			    CORBA_unsigned_long request_id,
			    GIOPReplyStatusType reply_status)
{
  GIOPSendBuffer *send_buffer;

  send_buffer = giop_send_buffer_new (connection);

  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "Sending reply id %u.", request_id);

  if (!send_buffer)
    return NULL;

  GIOP_MSG_TYPE (send_buffer) = GIOP_REPLY;

  giop_send_buffer_service_context_encode (send_buffer, service_context);

  GIOP_MSG_REPLY (send_buffer)->request_id = request_id;
  GIOP_MSG_REPLY (send_buffer)->reply_status = reply_status;

  giop_send_buffer_copy_atom (send_buffer, request_id);
  giop_send_buffer_copy_atom (send_buffer, reply_status);

  return send_buffer;
}

GIOPSendBuffer *
giop_send_buffer_new_locate_reply (GIOPConnection * connection,
				   CORBA_unsigned_long request_id,
				   GIOPLocateStatusType locate_reply_status)
{
  GIOPSendBuffer *send_buffer;

  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "Sending locate reply id %u.", 
	      request_id);

  send_buffer = giop_send_buffer_new (connection);

  if (!send_buffer)
    return NULL;

  GIOP_MSG_TYPE (send_buffer) = GIOP_LOCATEREPLY;

  giop_send_buffer_copy_atom (send_buffer, request_id);
  giop_send_buffer_copy_atom (send_buffer, locate_reply_status);

  return send_buffer;
}

GIOPSendBuffer *
giop_send_buffer_new_request (GIOPConnection * connection,
			      const IOP_ServiceContextList * service_context,
			      CORBA_unsigned_long request_id,
			      CORBA_boolean response_expected,
			      CORBA_sequence_octet * object_key,
			      guchar * operation,
			      gulong operation_len,
			      CORBA_sequence_octet * principal)
{
  GIOPSendBuffer *send_buffer;

  if (!connection || !object_key || !operation || !principal)
    return NULL;

  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "Sending request %s id %u to %s.",
	      operation, request_id, object_key->_buffer);

  send_buffer = giop_send_buffer_new (connection);

  if (!send_buffer)
    return NULL;

  GIOP_MSG_TYPE (send_buffer) = GIOP_REQUEST;

  giop_send_buffer_service_context_encode (send_buffer, service_context);

  GIOP_MSG_REQUEST (send_buffer)->request_id = request_id;
  GIOP_MSG_REQUEST (send_buffer)->response_expected = response_expected;

  giop_send_buffer_copy_atom (send_buffer, request_id);
  giop_send_buffer_copy_atom (send_buffer, response_expected);

  giop_send_buffer_put_sequence_octet (send_buffer, *object_key);

  giop_send_buffer_put_string (send_buffer, operation, operation_len);

  giop_send_buffer_put_sequence_octet (send_buffer, *principal);

  return send_buffer;
}

GIOPSendBuffer *
giop_send_buffer_new_locate_request (GIOPConnection * connection,
				     CORBA_unsigned_long request_id,
				     CORBA_sequence_octet * object_key)
{
  GIOPSendBuffer *send_buffer;

  if (!connection || !object_key)
    return NULL;

  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "Sending locate request id %u to %s.",
	      request_id, object_key->_buffer);

  send_buffer = giop_send_buffer_new (connection);

  if (!send_buffer)
    return NULL;

  GIOP_MSG_TYPE (send_buffer) = GIOP_LOCATEREQUEST;

  giop_send_buffer_copy_atom (send_buffer, request_id);

  giop_send_buffer_put_sequence_octet (send_buffer, *object_key);

  return send_buffer;
}

void
giop_send_buffer_free (GIOPSendBuffer * send_buffer)
{
  if (send_buffer == NULL)
    return;

  giop_connection_unref (GIOP_MSG_CONNECTION (send_buffer));

  G_LOCK (send_buffer_list);
  send_buffer_list = g_slist_prepend (send_buffer_list, send_buffer);
  G_UNLOCK (send_buffer_list);
}

void
giop_send_buffer_align_cross_border (GIOPSendBuffer * send_buffer,
				     gulong align_for)
{
  giop_send_buffer_append_open_indirect (send_buffer);
  send_buffer->indirect_cur = 0;
  send_buffer->indirect = g_chunk_new (gpointer, send_buffer->indirects);
  giop_send_buffer_align (send_buffer, align_for);
}

gpointer
giop_send_buffer_copy_mem_cross_border (GIOPSendBuffer * send_buffer,
					gconstpointer src,
					gulong len)
{
  gpointer first_object_address = NULL;
  glong remaining_space = GIOP_INDIRECT_CHUNK_SIZE - send_buffer->indirect_cur;
  glong to_copy;
  g_assert (remaining_space >=0);

  if (remaining_space >= 8)
    {
      first_object_address = PTR_ADD (send_buffer->indirect, 
				      send_buffer->indirect_cur);

      to_copy = MIN (remaining_space, len);
      giop_send_buffer_copy_mem (send_buffer, src, to_copy);
      src = PTR_ADD (src, to_copy);
      len -= to_copy;	
    }

  while (len > 0)
    {
      giop_send_buffer_append_open_indirect (send_buffer);
      send_buffer->indirect_cur = 0;
      send_buffer->indirect = g_chunk_new (gpointer, send_buffer->indirects);

      if (!first_object_address)
	first_object_address = send_buffer->indirect;

      to_copy = MIN (GIOP_INDIRECT_CHUNK_SIZE, len);
      giop_send_buffer_copy_mem (send_buffer, src, to_copy);
      src = PTR_ADD (src, to_copy);
      len -= to_copy;	
    }

  return first_object_address;
}
