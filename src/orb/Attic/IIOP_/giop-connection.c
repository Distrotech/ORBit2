#include "IIOP-private.h"

#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

/* internal functions */

static gboolean giop_handle_connection (GIOChannel * source,
					GIOCondition condition,
					gpointer connection_data);

static void default_add_connection (GIOPConnection * connection);
static void default_remove_connection (GIOPConnection * connection);
static GIOPRecvBuffer *default_wait_for_reply (GIOPConnection *connection,
					       GArray *request_ids,
					       gboolean block_for_reply);
static void default_notify_reply (GIOPRecvBuffer *recv_buffer,
				       CORBA_unsigned_long id);
static gint giop_connection_compare (gconstpointer in_connection, 
				     gconstpointer in_info);

/* global variables */

#define default_main_loop_iteration ((GIOPMainLoopIteration)g_main_iteration);

gboolean giop_unlink_unix_sockets_atexit = TRUE;

static GIOPAddConnectionFunc add_connection_func = default_add_connection;
static GIOPRemoveConnectionFunc remove_connection_func = 
                                                 default_remove_connection;
static GIOPMainLoopIteration main_loop_iteration_func =
                                                 default_main_loop_iteration;
static GIOPIncomingMessageFunc incoming_message_func = NULL;
static GIOPWaitForReplyFunc wait_for_reply_func = default_wait_for_reply;
static GIOPNotifyReplyFunc notify_reply_func = default_notify_reply;

G_LOCK_DEFINE_STATIC (giop_connection_list);
static GList *giop_connection_list = NULL;

G_LOCK_DEFINE_STATIC (giop_reply_list);
static GList *giop_reply_list = NULL;

static GMainLoop *giop_main_loop = NULL;

/**
 * giop_init: initialize GIOP subsystem.
 * @argv0: Name of the program as needed for TCP Wrappers
 * @functions: Customization of the behaviour of the GIOP subsystem
 *
 * Initializes giop_connection_list. Calls giop_message_buffer_init()
 * to initialize the message_buffer subsystem. Calls iiop_init() to
 * perform IIOP-specific initialization.
 **/
void
giop_init (const char *argv0)
{
  struct sigaction mypipe;
  g_assert (sizeof (GIOPMessageHeader) == 12);

  memset (&mypipe, '\0', sizeof (mypipe));
  mypipe.sa_handler = SIG_IGN;

  sigaction (SIGPIPE, &mypipe, NULL);

  giop_main_loop = g_main_new (FALSE);

  giop_connection_list = NULL;

  /*
   * This also needs to do any transport-specific initialization
   * as appropriate
   */
  iiop_init (argv0);
}

void 
giop_set_main_loop_handler(GIOPAddConnectionFunc add_connection,
			   GIOPRemoveConnectionFunc remove_connection,
			   GIOPMainLoopIteration main_loop_iteration)
{ 
  if (add_connection && remove_connection && main_loop_iteration)
    {
      add_connection_func = add_connection;
      remove_connection_func = remove_connection;
      main_loop_iteration_func = main_loop_iteration;
    }
  else if (!add_connection && !remove_connection && !main_loop_iteration)
    {
      add_connection_func = default_add_connection;
      remove_connection_func = default_remove_connection;
      main_loop_iteration_func = default_main_loop_iteration;
    }
  else
    {
      ORBit_warning (ORBIT_LOG_DOMAIN_IIOP, 
		     "`add_connection', `remove_connection' and "
		     "`main_loop_iteration' functions must \n"
		     "either be all set or all unset. Using old values.");
    }
}

void 
giop_set_incoming_message_handler(GIOPIncomingMessageFunc handle_incoming)
{
  incoming_message_func = handle_incoming;
}

void 
giop_set_reply_handler(GIOPWaitForReplyFunc wait_for_reply,
		       GIOPNotifyReplyFunc notify_reply)
{
  if (wait_for_reply && notify_reply)
    {
      wait_for_reply_func = wait_for_reply;
      notify_reply_func = notify_reply;
    }
  else if (!wait_for_reply && !notify_reply)
    {
      wait_for_reply_func = default_wait_for_reply;
      notify_reply_func = default_notify_reply;
    }
  else
    {
      ORBit_warning (ORBIT_LOG_DOMAIN_IIOP,
		     "`wait_for_reply' and `notify_reply' functions must \n"
		     "either be both set or both unset. Using old values.");
    }
}


/*** giop_connection_init
 *
 *   Inputs: 'giop_connection' - memory region allocated for use as a
 *                               GIOPConnection.
 *           'cnxclass'        - the class of connection that will be stored
 *                               here (SERVER, CLIENT)
 *
 *   Outputs: None
 *
 *   Side effects: Initializes 'giop_connection'.
 *
 *   Description: Basic setup of a GIOPConnection.
 *                Sets is_valid to FALSE because it is the responsibility of
 *	          the transport-specific initialization routine to make
 *	          a connection valid.
 */

void
giop_connection_init (GIOPConnection * connection,
		      GQuark type,
		      GIOPConnectionClass cnxclass)
{
  connection->type = type;
  connection->refcount = 0;
  connection->class = cnxclass;
  connection->is_valid = FALSE;
  connection->is_auth = FALSE;
  connection->was_initiated = FALSE;

  connection->orb_data = NULL;
  connection->user_data = NULL;
  connection->user_data_destroy_func = NULL;

  ORBit_debug (ORBIT_LOG_DOMAIN_IIOP, "new GIOPConnection.");
}

/*
 * giop_connection_free
 *    Inputs: 'connection'
 *    Outputs: None
 *    Side effects: Makes the 'connection' invalid as a GIOPConnection
 *                  and as a gpointer.
 *
 *    Description: Calls giop_connection_remove_from_list() to
 *                 stop the connection from being used for incoming.
 *
 *	           If a transport-specific finalization function has
 *	           been provided, call it.
 *	           
 *	           Free the memory block at '*connection'.
 *
 */
void
giop_connection_free (GIOPConnection * connection)
{
  g_return_if_fail (connection != NULL);

  ORBit_debug (ORBIT_LOG_DOMAIN_IIOP, "destroy GIOPConnection fd %d.", 
	       giop_connection_fd (connection));

  if (!connection->was_initiated)
    {
      const GIOPMessageHeader close_connection = 
      { "GIOP", {1, 0}, GIOP_FLAG_ENDIANNESS, GIOP_CLOSECONNECTION, 0 };
      GIOPIovecArray *iovecs =  giop_iovec_array_new ();
      
      ORBit_info (ORBIT_LOG_DOMAIN_IIOP,
		  "sending GIOP_CLOSECONNECTION to peer fd %d.",
		  giop_connection_fd (connection));

      giop_iovec_array_append (iovecs, &close_connection, 
			       sizeof (close_connection));
      
      giop_connection_writev (connection, iovecs, sizeof (close_connection));
      
      giop_iovec_array_free (iovecs);
    }

  if (connection->destroy_func)
    {      
      connection->destroy_func (connection);
    }

  g_io_channel_close (connection->channel);
  g_io_channel_unref (connection->channel);

  if (connection->user_data && connection->user_data_destroy_func)
    connection->user_data_destroy_func (connection->user_data);

  g_free (connection);
}

static void 
default_add_connection (GIOPConnection * connection)
{
  connection->tag = g_io_add_watch (connection->channel, G_IO_PRI | G_IO_IN | 
				    G_IO_ERR | G_IO_HUP | G_IO_NVAL, 
				    giop_handle_connection, connection);
}

static void 
default_remove_connection (GIOPConnection * connection)
{
  g_source_remove (connection->tag);
}

/*
 * giop_connection_add_to_list
 *
 *    Inputs: 'connection' - a GIOPConnection that the user wishes added to the list
 *    Outputs: None
 *
 *    Side effects: Modifies giop_connection_list
 *    Global data structures used: giop_connection_list
 *    Bugs: Does not check for duplicate additions.
 *
 *    Description:
 *         Adds a connection to the list of active connections.
 */
void
giop_connection_add_to_list (GIOPConnection * connection)
{
  g_return_if_fail (connection->is_valid == FALSE);

  ORBit_debug (ORBIT_LOG_DOMAIN_IIOP, "adding GIOPConnection fd %d.",
	       giop_connection_fd (connection));

  connection->is_valid = TRUE;

  G_LOCK (giop_connection_list);
  giop_connection_list = g_list_prepend (giop_connection_list, connection);
  G_UNLOCK (giop_connection_list);

  add_connection_func (connection);
  giop_connection_ref (connection);
}

/*
 * giop_connection_remove_from_list
 *
 *    Inputs: 'connection' - a GIOPConnection that the user wishes
 *    Outputs: None
 *
 *    Side effects: Modifies giop_connection_list
 *    Global data structures used: giop_connection_list
 *
 *    Description:
 *         Removes a connection from the list of active connections.
 *         Calls the library user's "I removed connection" handler if it
 *         exists.
 *
 *    Bugs: Does not check for duplicate removals. This may not be "bad" though.
 */
void
giop_connection_remove_from_list (GIOPConnection * connection)
{
  GList *link;

  G_LOCK (giop_connection_list);
  link = g_list_find (giop_connection_list, connection);
  G_UNLOCK (giop_connection_list);

  if (link)
    {
      ORBit_debug (ORBIT_LOG_DOMAIN_IIOP, "removing GIOPConnection fd %d.",
		   giop_connection_fd (connection));

      remove_connection_func (connection);
      
      G_LOCK (giop_connection_list);
      /* FIXME: use g_list_delete_link, once in GLib */
      giop_connection_list = g_list_remove_link (giop_connection_list, link);
      g_list_free_1 (link);
      G_UNLOCK (giop_connection_list);

      giop_connection_unref (connection);
    }
}

void
giop_connection_remove_by_orb(gpointer match_orb_data)
{
  GList *link;

  G_LOCK (giop_connection_list);
  for (link = giop_connection_list; link; ) 
    {
      GIOPConnection *cnx = link->data; 
      link = link->next;
      if (cnx->orb_data == match_orb_data) 
	{
	  giop_connection_remove_from_list (cnx);
	} 
    } 
  G_UNLOCK (giop_connection_list);
}

gint
giop_connection_compare (gconstpointer in_connection, gconstpointer in_info)
{
  const GIOPConnection *connection = in_connection;
  const struct { GQuark type; gpointer data; 
    GCompareFunc func; } *info = in_info;

  if(connection->type != info->type)
    return -1;

  if(!connection->is_valid)
    return -1;
  
  return info->func (connection, info->data);
}

GIOPConnection *
giop_connection_find_in_list(GQuark type,
			     gpointer data, 
			     GCompareFunc func)
{
  GList *found;
  struct { GQuark type; gpointer data; GCompareFunc func; } info;

  g_return_val_if_fail(func, NULL);

  info.type = type;
  info.data = data;
  info.func = func;
  
  G_LOCK (giop_connection_list);
  found = g_list_find_custom (giop_connection_list, &info, 
			      giop_connection_compare);
  G_UNLOCK (giop_connection_list);

  return found ? found->data : NULL;
}

void
giop_main_quit (void)
{
  g_assert (giop_main_loop);
  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "quiting main loop.");
  g_main_quit (giop_main_loop);
}

void
giop_main (void)
{
  g_assert (giop_main_loop);
  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "entering main loop.");
  g_main_run (giop_main_loop);
}

GIOPRecvBuffer *
giop_connection_wait_for_reply_multiple(GIOPConnection *connection,
					GArray *request_ids,
					gboolean block_for_reply)
{
  GIOPRecvBuffer *retval = NULL;

  retval = wait_for_reply_func (connection, request_ids, block_for_reply);
  
  g_assert (!retval || GIOP_MSG_TYPE (retval) == GIOP_REPLY);

  return retval;
}

GIOPRecvBuffer *
giop_connection_wait_for_reply(GIOPConnection *connection,
			       CORBA_unsigned_long request_id,
			       gboolean block_for_reply)
{
  GArray array;

  array.len = 1;
  array.data = (gpointer)&request_id;

  return giop_connection_wait_for_reply_multiple(connection, &array,
						 block_for_reply);
}

GIOPRecvBuffer *
giop_connection_wait_for_locate_reply(GIOPConnection *connection,
				      CORBA_unsigned_long request_id,
				      gboolean block_for_reply)
{
  GIOPRecvBuffer *retval = NULL;
  GArray array;

  array.len = 1;
  array.data = (gpointer)&request_id;

  retval = wait_for_reply_func (connection, &array, block_for_reply);
  
  g_assert (!retval || GIOP_MSG_TYPE (retval) == GIOP_LOCATEREPLY);

  return retval;
}

static gint 
compare_to_request_ids(gconstpointer data, gconstpointer array)
{
  const GArray* request_id_array = array;
  CORBA_unsigned_long request_id = giop_recv_buffer_reply_request_id (data);
  int i;

  for(i = 0; i < request_id_array->len; i++)
    {
      if(request_id == g_array_index(request_id_array, CORBA_unsigned_long, i))
        return 0;
    }
  return -1;
}

static GIOPRecvBuffer *
default_wait_for_reply (GIOPConnection *connection, 
			     GArray *request_ids,
			     gboolean block_for_reply)
{
  do
    {
      GList* found;
      
      G_LOCK (giop_reply_list);      
      found = g_list_find_custom( giop_reply_list, request_ids, 
                                  compare_to_request_ids);
      if (found)
        {
          GIOPRecvBuffer *retval = found->data;
	  /* FIXME: use g_list_delete_link, once in GLib */
          giop_reply_list = g_list_remove_link( giop_reply_list, found );
	  g_list_free_1 (found);
          G_UNLOCK (giop_reply_list);      
          return retval;
        }
      G_UNLOCK (giop_reply_list);      

      if (connection && !connection->is_valid) return NULL;

      main_loop_iteration_func (block_for_reply);

    } while (block_for_reply);
  return NULL;  
}

static void 
default_notify_reply (GIOPRecvBuffer *recv_buffer,
			   CORBA_unsigned_long id)
{
  G_LOCK (giop_reply_list);
  giop_reply_list = g_list_prepend (giop_reply_list, recv_buffer);
  G_UNLOCK (giop_reply_list);
}

static gboolean
giop_handle_connection (GIOChannel * source,
			GIOCondition condition,
			gpointer connection_data)
{
  GIOPConnection *connection = connection_data;

  g_return_val_if_fail (connection != NULL, FALSE);
  g_return_val_if_fail (connection->is_valid, FALSE);

#ifdef ORBIT_DEBUG
  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "event on connection fd %d:",
	      giop_connection_fd (connection));
  if (condition & G_IO_IN)
    ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "    G_IO_IN");
  if (condition & G_IO_PRI)
    ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "    G_IO_PRI");
  if (condition & G_IO_ERR)
    ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "    G_IO_ERR");
  if (condition & G_IO_HUP)
    ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "    G_IO_HUP");
  if (condition & G_IO_NVAL)
    ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "    G_IO_NVAL");
#endif
  if (condition & G_IO_IN)
    {
      GIOPRecvBuffer *recv_buffer;

      if (connection->class == GIOP_CONNECTION_SERVER)
	{
	  g_assert (connection->accept_func);
	  connection->accept_func (connection);
	  return TRUE;
	}
      
      recv_buffer = giop_recv_buffer_read (connection);

      if (recv_buffer)
	{
	  CORBA_unsigned_long request_id = 
	    giop_recv_buffer_reply_request_id(recv_buffer);
	  if (request_id == 0)
	    {
	      if (incoming_message_func)
		incoming_message_func (recv_buffer);
	    }
	  else 
	    notify_reply_func (recv_buffer, request_id);
	}
      return TRUE;
    }
  /* now it is one of  G_IO_PRI | G_IO_ERR | G_IO_HUP | G_IO_NVAL */
  giop_connection_invalidate (connection);
  return FALSE;
}

/*
 * giop_connection_invalidate
 *
 * Input: GIOPConnection *connection
 *
 * Output:
 *
 * Side effects: invalidates connection
 */

void
giop_connection_invalidate (GIOPConnection * connection)
{
  g_return_if_fail (connection != NULL);

  giop_connection_ref (connection);

  giop_connection_remove_from_list (connection);

  connection->is_valid = FALSE;

  if (connection->incoming_msg)
    {
      giop_recv_buffer_free (connection->incoming_msg);
      connection->incoming_msg = NULL;
    }

  giop_connection_unref (connection);
}

glong
giop_connection_read (GIOPConnection * connection, gpointer buf, gulong len)
{
  guint bytes_read;
  GIOError error = g_io_channel_read (connection->channel, buf, len, 
				      &bytes_read);

  if (error == G_IO_ERROR_AGAIN)
    {
      /* This is not really an error, so we just return 0 */
      return 0;
    }
  else if (error != G_IO_ERROR_NONE)
    {
      /* There is a real error and nothing should be read anymore from
       * this channel */
      ORBit_critical (ORBIT_LOG_DOMAIN_IIOP, "iiop_connection_read: "
		      "error while reading from fd %d: %s",
		      giop_connection_fd (connection), 
		      g_strerror (errno));
      return -1;
    }
  else
    ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "Read %d bytes from fd %d.", 
		bytes_read, giop_connection_fd (connection));
  
  return bytes_read;
}

#ifdef HAVE_LIMITED_WRITEV
#define writev g_writev
#endif

gboolean
giop_connection_writev (GIOPConnection * connection,
			GArray * iovecs, gulong len)
{
  gulong nvecs = iovecs->len;
  glong bytes_written;
  struct iovec *curvec;
  int fd;

  if (!connection->is_valid)
    return FALSE;

  fd = giop_connection_fd (connection);

  curvec = (struct iovec *) iovecs->data;

#if defined(ORBIT_DEBUG)
  ORBit_info (ORBIT_LOG_DOMAIN_IIOP, "Writing %lu bytes to fd %d", len, fd);
  ORBit_debug (ORBIT_LOG_DOMAIN_IIOP, "  Message looks like:");
  {
    gulong i, sum = 0;
    for (i = 0; i < nvecs; i++)
      {
	sum += curvec[i].iov_len;
	ORBit_debug (ORBIT_LOG_DOMAIN_IIOP, "    [%p, %u]: %lu.", 
		     curvec[i].iov_base, curvec[i].iov_len, sum);
      }
  }
#endif

  bytes_written = writev (fd, curvec, nvecs);

  if (bytes_written < len)
    {
      glong t = 0;
      if (bytes_written < 0)
	{
	  if (errno != EAGAIN)
	    {
	      giop_connection_invalidate (connection);
	      return FALSE;
	    }
	  bytes_written = 0;
	}

      while (1)
	{
	  if ((t + curvec->iov_len) > bytes_written)
	    break;
	  t += curvec->iov_len;
	  curvec++;
	  nvecs--;
	}

      if ((bytes_written - t) > 0)
	{
	  curvec->iov_len -= (bytes_written - t);
	  curvec->iov_base = (gpointer) ((char *) curvec->iov_base
					 + (bytes_written - t));
	}

      fcntl (fd, F_SETFL, fcntl (fd, F_GETFL, 0) & ~O_NONBLOCK);

      t = writev (fd, curvec, nvecs);

      fcntl (fd, F_SETFL, fcntl (fd, F_GETFL, 0) | O_NONBLOCK);

      if ((t < 0) || (bytes_written + t < len))
	{
	  giop_connection_invalidate (connection);
	  return FALSE;
	}
    }
  return TRUE;
}
