#include "IIOP-private.h"

#include <string.h>

/* local functions */
static gboolean giop_recv_buffer_decode_message (GIOPRecvBuffer * buf);
static gboolean giop_recv_buffer_decode_reply_message (GIOPRecvBuffer * buf);
static gboolean giop_recv_buffer_decode_request_message (GIOPRecvBuffer * buf);
static gboolean giop_recv_buffer_decode_locate_reply_message 
                                                     (GIOPRecvBuffer * buf);
static gboolean giop_recv_buffer_decode_locate_request_message 
                                                     (GIOPRecvBuffer * buf);
static gboolean giop_recv_buffer_decode_service_context 
                      (GIOPRecvBuffer * buf, IOP_ServiceContextList * context);

G_LOCK_DEFINE_STATIC (recv_buffer_list);
static GSList *recv_buffer_list = NULL;

static GIOPRecvBuffer *
giop_recv_buffer_new (void)
{
  GIOPRecvBuffer *msgbuf;

  msgbuf = g_new (GIOPRecvBuffer, 1);

  msgbuf->message = NULL;

  return msgbuf;
}

void
giop_recv_buffer_free (GIOPRecvBuffer * buffer)
{
  if (buffer == NULL)
    return;

  if (buffer->message)
    {
      g_free (buffer->message);
      buffer->message = NULL;
    }

  if (GIOP_MSG_CONNECTION (buffer)->incoming_msg == buffer)
    GIOP_MSG_CONNECTION (buffer)->incoming_msg = NULL;

  giop_connection_unref (GIOP_MSG_CONNECTION (buffer));

  G_LOCK (recv_buffer_list);
  recv_buffer_list = g_slist_prepend (recv_buffer_list, buffer);
  G_UNLOCK (recv_buffer_list);
}

GIOPRecvBuffer *
giop_recv_buffer_read (GIOPConnection * connection)
{
  GIOPRecvBuffer *recv_buffer = NULL;
  gpointer *read_buffer;
  int bytes_read;

  if (!connection || !connection->is_valid)
    return NULL;

  if (connection->incoming_msg)
    recv_buffer = connection->incoming_msg;
  else
    {
      G_LOCK (recv_buffer_list);

      if (recv_buffer_list)
	{
	  GSList *head;

	  recv_buffer = recv_buffer_list->data;

	  head = recv_buffer_list;
	  /* FIXME: use g_slist_delete_link, once in GLib */
	  recv_buffer_list = g_slist_remove_link (recv_buffer_list,
						  recv_buffer_list);
	  g_slist_free_1 (head);

	  GIOP_MSG_SIZE (recv_buffer) = 0;
	  recv_buffer->message = NULL;
	}
      else
	recv_buffer = giop_recv_buffer_new ();

      G_UNLOCK (recv_buffer_list);

      recv_buffer->state = GIOP_RECV_BUFFER_READING_HEADER;
      recv_buffer->left_to_read = sizeof (GIOPMessageHeader);

      giop_connection_ref (connection);
      GIOP_MSG_CONNECTION (recv_buffer) = connection;

      connection->incoming_msg = recv_buffer;
    }

  g_assert (recv_buffer);

  do
    {
      switch (recv_buffer->state)
	{
	case GIOP_RECV_BUFFER_READING_HEADER:
	  read_buffer = PTR_ADD (GIOP_MSG_HEADER (recv_buffer),
				 sizeof (GIOPMessageHeader));
	  break;
	case GIOP_RECV_BUFFER_READING_BODY:
	  read_buffer = PTR_ADD (GIOP_RECV_BUFFER_BODY (recv_buffer),
				 GIOP_MSG_SIZE (recv_buffer));
	  break;
	default:
	  read_buffer = NULL;	/* To make 'gcc -Wall' happy */
	  g_assert_not_reached ();
	  break;
	}

      read_buffer = PTR_SUB (read_buffer, recv_buffer->left_to_read);

      bytes_read = giop_connection_read (connection, read_buffer,
					 recv_buffer->left_to_read);

      if (bytes_read < 0)
	{
	  ORBit_critical (ORBIT_LOG_DOMAIN_IIOP, "giop_recv_buffer_read: "
			  "expected to read %d bytes. Got nothing.", 
			  recv_buffer->left_to_read);
	  goto errout;
	}
      else if (bytes_read == 0)
	{
	  return NULL;
	}
      
      recv_buffer->left_to_read -= bytes_read;

      if (recv_buffer->left_to_read == 0)
	{
	  switch (recv_buffer->state)
	    {
	    case GIOP_RECV_BUFFER_READING_HEADER:
	      /* Check the magic stuff */
	      if (strncmp (GIOP_MSG_HEADER (recv_buffer)->magic, "GIOP", 4) ||
		  GIOP_MSG_HEADER (recv_buffer)->GIOP_version[0] != 1)
		{
		  ORBit_critical (ORBIT_LOG_DOMAIN_IIOP,
				  "giop_recv_buffer_read: Got invalid magic "
				  "header: %c%c%c%c %d.%d",
				  GIOP_MSG_HEADER (recv_buffer)->magic[0],
				  GIOP_MSG_HEADER (recv_buffer)->magic[1],
				  GIOP_MSG_HEADER (recv_buffer)->magic[2],
				  GIOP_MSG_HEADER (recv_buffer)->magic[3],
				  GIOP_MSG_HEADER (recv_buffer)->
				  GIOP_version[0],
				  GIOP_MSG_HEADER (recv_buffer)->
				  GIOP_version[1]);
		  goto errout;
		}

	      if (GIOP_MSG_SIZE (recv_buffer) == 0 &&
		  GIOP_MSG_TYPE (recv_buffer) != GIOP_CLOSECONNECTION)
		{
		  ORBit_critical (ORBIT_LOG_DOMAIN_IIOP,
				  "giop_recv_buffer_read: Unexpected 0-length "
				  "IIOP message of type %d.", 
				  GIOP_MSG_TYPE (recv_buffer));
		  goto errout;
		}

	      if (giop_recv_buffer_conversion_needed (recv_buffer))
		{
		  GIOP_MSG_SIZE (recv_buffer) =
		    GUINT32_SWAP_LE_BE (GIOP_MSG_SIZE (recv_buffer));
		}

	      if (!connection->is_auth
		  && GIOP_MSG_SIZE (recv_buffer) > 131072)
		{
		  ORBit_critical (ORBIT_LOG_DOMAIN_IIOP, 
				  "giop_recv_buffer_read: message size is "
				  "bigger than 128k (%d).",
				  GIOP_MSG_SIZE (recv_buffer));
		  goto errout;
		}

	      recv_buffer->message =
		g_malloc (GIOP_MSG_REAL_SIZE (recv_buffer));
	      recv_buffer->state = GIOP_RECV_BUFFER_READING_BODY;
	      recv_buffer->left_to_read = GIOP_MSG_SIZE (recv_buffer);
	      break;
	    case GIOP_RECV_BUFFER_READING_BODY:
	      if (!giop_recv_buffer_decode_message (recv_buffer))
		{
		  ORBit_critical (ORBIT_LOG_DOMAIN_IIOP, 
				  "giop_recv_buffer_read: couldn't decode "
				  "message");
		  return NULL;
		}
	      connection->incoming_msg = NULL;
	      recv_buffer->state = GIOP_RECV_BUFFER_READY;
	      break;
	    default:
	      g_assert_not_reached ();
	      break;
	    }
	}
      else if (recv_buffer->left_to_read < 0)
	{
	  ORBit_critical (ORBIT_LOG_DOMAIN_IIOP, 
			  "giop_recv_buffer_read: the message is %d bytes "
			  "bigger than supposed to be. ",
			  -recv_buffer->left_to_read);
	  goto errout;
	}
      else /* recv_buffer->left_to_read > 0 */
	{
	  /* couldn't read the whole piece, save it */
	  recv_buffer = NULL;
	}
    }
  while (recv_buffer && recv_buffer->state != GIOP_RECV_BUFFER_READY);

  return recv_buffer;

errout:
  giop_recv_buffer_free (recv_buffer);
  giop_connection_invalidate (connection);
  return NULL;
}

static gboolean
giop_recv_buffer_decode_message (GIOPRecvBuffer * buf)
{
  buf->cur = GIOP_RECV_BUFFER_BODY (buf);
  switch (GIOP_MSG_TYPE (buf))
    {
    case GIOP_REPLY:
      return giop_recv_buffer_decode_reply_message (buf);
      break;
    case GIOP_REQUEST:
      return giop_recv_buffer_decode_request_message (buf);
      break;
    case GIOP_LOCATEREQUEST:
      return giop_recv_buffer_decode_locate_request_message (buf);
      break;
    case GIOP_LOCATEREPLY:
      return giop_recv_buffer_decode_locate_reply_message (buf);
      break;
    case GIOP_CLOSECONNECTION:
      return TRUE;
      break;
    default:
      ORBit_critical (ORBIT_LOG_DOMAIN_IIOP, 
		      "giop_recv_buffer_decode_message: Unknown message "
		      "type %d.", GIOP_MSG_TYPE (buf));
      return FALSE;
    }
}

static gboolean
giop_recv_buffer_decode_service_context (GIOPRecvBuffer * buf,
					 IOP_ServiceContextList * context)
{
  gint i;
  /* The minimal packed contents of an IOP_ServiceContext is the ID of
   * this service context, plus the length of the corresponding octet
   * string in bytes  */
  static const guint packed_len_of_IOP_ServiceContext =
    sizeof (IOP_ServiceId) + sizeof (CORBA_unsigned_long);

  context->_maximum = 0;
  giop_recv_buffer_get_atom (buf, context->_length);

  if (GIOP_MSG_SIZE (buf) < 
      context->_length * packed_len_of_IOP_ServiceContext)
     /* The IOP_ServiceContextList of the give length can't possibly
      * fit into the length of the message, thus we have an error. */
     return FALSE;

  context->_buffer = g_new (IOP_ServiceContext, context->_length);

  for (i = 0; i < context->_length; i++)
    {
      giop_recv_buffer_get_atom (buf, context->_buffer[i].context_id);
      giop_recv_buffer_get_sequence_octet (buf,
					   context->_buffer[i].context_data);
    }

  return !giop_recv_buffer_overstepped (buf);
}

static gboolean
giop_recv_buffer_decode_reply_message (GIOPRecvBuffer * buf)
{
  if (!giop_recv_buffer_decode_service_context 
      (buf, &(GIOP_MSG_REPLY (buf)->service_context)))
    return FALSE;

  giop_recv_buffer_get_atom (buf, GIOP_MSG_REPLY (buf)->request_id);
  giop_recv_buffer_get_atom (buf, GIOP_MSG_REPLY (buf)->reply_status);

  if (giop_recv_buffer_overstepped (buf))
      return FALSE;

  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, 
	      "Received reply with status %d and size %d to request %d.", 
	      GIOP_MSG_REPLY (buf)->reply_status,
	      GIOP_MSG_SIZE (buf), GIOP_MSG_REPLY (buf)->request_id);
  return TRUE;
}

static gboolean
giop_recv_buffer_decode_locate_reply_message (GIOPRecvBuffer * buf)
{
  giop_recv_buffer_get_atom (buf, GIOP_MSG_LOCATEREPLY (buf)->request_id);
  giop_recv_buffer_get_atom (buf, GIOP_MSG_LOCATEREPLY (buf)->locate_status);

  if (giop_recv_buffer_overstepped (buf))
      return FALSE;

  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "Received locate reply with status %d "
	      "and size %d to request %d.", 
	      GIOP_MSG_LOCATEREPLY (buf)->locate_status,
	      GIOP_MSG_SIZE (buf), GIOP_MSG_LOCATEREPLY (buf)->request_id);
  return TRUE;
}

static gboolean
giop_recv_buffer_decode_request_message (GIOPRecvBuffer * buf)
{
  CORBA_unsigned_long len;

  if (!giop_recv_buffer_decode_service_context 
      (buf, &(GIOP_MSG_REQUEST (buf)->service_context)))
    return FALSE;

  giop_recv_buffer_get_atom (buf, GIOP_MSG_REQUEST (buf)->request_id);
  giop_recv_buffer_get_atom (buf, GIOP_MSG_REQUEST (buf)->response_expected);

  giop_recv_buffer_get_sequence_octet (buf,
				       GIOP_MSG_REQUEST (buf)->object_key);

  giop_recv_buffer_get_string (buf, GIOP_MSG_REQUEST (buf)->operation, len);

  giop_recv_buffer_get_sequence_octet (buf,
				       (GIOP_MSG_REQUEST (buf)->
					requesting_principal));

  if (giop_recv_buffer_overstepped (buf))
      return FALSE;

  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "Received request '%s' with size %d "
	      "and id %d.", GIOP_MSG_REQUEST (buf)->operation,
	      GIOP_MSG_SIZE (buf), GIOP_MSG_REQUEST (buf)->request_id);

  return TRUE;
}

static gboolean
giop_recv_buffer_decode_locate_request_message (GIOPRecvBuffer * buf)
{
  giop_recv_buffer_get_atom (buf, GIOP_MSG_LOCATEREQUEST (buf)->request_id);
  giop_recv_buffer_get_sequence_octet (buf,
				       (GIOP_MSG_LOCATEREQUEST (buf)->
					object_key));

  if (giop_recv_buffer_overstepped (buf))
      return FALSE;

  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, 
	      "Received locate request with size %d and id %d.",
	      GIOP_MSG_SIZE (buf), GIOP_MSG_LOCATEREQUEST (buf)->request_id);

  return TRUE;
}
