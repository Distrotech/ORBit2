#ifndef GIOP_CONNECTION_H
#define GIOP_CONNECTION_H 1

#include <IIOP/giop-types.h>
#include <ORBitutil/trace.h>
#include <sys/uio.h> /* For struct iovec */

enum _GIOPConnectionClass
{
  /* Not a real connection to any place - just listening */
  GIOP_CONNECTION_SERVER,
  GIOP_CONNECTION_CLIENT
};

#define GIOP_CONNECTION(x) ((GIOPConnection *)(x))
struct _GIOPConnection
{
  GQuark type;

  void (*destroy_func) (GIOPConnection * connection);
  void (*accept_func) (GIOPConnection * connection);

  guint refcount;
  GIOPConnectionClass class;

  GIOChannel *channel;
  guint tag;

  /* You can access these if you wish. */
  gpointer orb_data;

  gpointer user_data;
  GDestroyNotify user_data_destroy_func;
  /* end accessable stuff */

  gboolean is_valid, was_initiated, is_auth;

  GIOPRecvBuffer *incoming_msg;
};

#define giop_connection_fd(x) (g_io_channel_unix_get_fd ((x)->channel))

#define giop_connection_ref(x)						\
G_STMT_START{								\
  (x)->refcount++; 							\
  ORBit_debug(ORBIT_LOG_DOMAIN_IIOP, "Reffing fd %d to %d",		\
	      giop_connection_fd (x), (x)->refcount); 			\
}G_STMT_END

#define giop_connection_unref(x) 					\
G_STMT_START{ 								\
  (x)->refcount--; 							\
  ORBit_debug(ORBIT_LOG_DOMAIN_IIOP, "Dereffing fd %d to %d",		\
              giop_connection_fd (x), (x)->refcount);			\
  if((x)->refcount <= 0) giop_connection_free(x); 			\
}G_STMT_END

/* must be set before calling giop_init; defaults to TRUE */
extern gboolean giop_unlink_unix_sockets_atexit;

void giop_set_main_loop_handler(GIOPAddConnectionFunc add_connection,
				GIOPRemoveConnectionFunc remove_connection,
				GIOPMainLoopIteration main_loop_iteration);

void giop_set_incoming_message_handler(GIOPIncomingMessageFunc 
				       handle_incoming);
void giop_set_reply_handler(GIOPWaitForReplyFunc wait_for_reply,
			    GIOPNotifyReplyFunc notify_reply);

void giop_init (const char *argv0);
void giop_main (void);
void giop_main_quit (void);

void giop_connection_free (GIOPConnection * connection);
void giop_connection_invalidate (GIOPConnection * connection);

GIOPRecvBuffer *giop_connection_wait_for_reply_multiple (GIOPConnection *
							 connection,
							 GArray * request_ids,
							 gboolean
							 block_for_reply);
GIOPRecvBuffer *giop_connection_wait_for_reply (GIOPConnection * connection,
						CORBA_unsigned_long request_id,
						gboolean block_for_reply);
GIOPRecvBuffer *giop_connection_wait_for_locate_reply (GIOPConnection *
						       connection,
						       CORBA_unsigned_long
						       request_id,
						       gboolean
						       block_for_reply);
void giop_connection_add_to_list (GIOPConnection * cnx);
void giop_connection_remove_from_list (GIOPConnection * cnx);
void giop_connection_remove_by_orb (gpointer match_orb_data);
GIOPConnection *giop_connection_find_in_list(GIOPConnectionClass class,
					     gpointer data, 
					     GCompareFunc func);
void giop_connection_init (GIOPConnection * giop_connection,
			   GQuark type,
			   GIOPConnectionClass cnxclass);

glong giop_connection_read (GIOPConnection * connection, 
			    gpointer buf, gulong len);

gboolean giop_connection_writev (GIOPConnection * connection,
				 GArray * iovecs, gulong len);

#define giop_iovec_array_new() 						\
   (g_array_new (FALSE, FALSE, sizeof (struct iovec)))
#define giop_iovec_array_append(array, mem, length)			\
G_STMT_START{								\
  struct iovec *new_iovec;						\
  g_array_set_size ((array), (array)->len + 1);				\
  new_iovec = &g_array_index ((array), struct iovec, (array)->len - 1);	\
  new_iovec->iov_base = (gpointer) (mem);				\
  new_iovec->iov_len = (length);					\
}G_STMT_END
#define giop_iovec_array_reset(array) 					\
   (g_array_set_size ((array), 0))
#define giop_iovec_array_free(array) 					\
   (g_array_free ((array), TRUE))

#endif /* GIOP_CONNECTION_H */
