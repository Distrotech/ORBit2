/*
 * linc.c: This file is part of the linc library.
 *
 * Authors:
 *    Elliot Lee     (sopwith@redhat.com)
 *    Michael Meeks  (michael@ximian.com)
 *    Mark McLouglin (mark@skynet.ie) & others
 *
 * Copyright 2001, Red Hat, Inc., Ximian, Inc.,
 *                 Sun Microsystems, Inc.
 */
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include "linc-debug.h"
#include "linc-private.h"

static gboolean link_threaded = FALSE;
static gboolean link_mutex_new_called = FALSE;
GMainLoop      *link_loop = NULL;
GMainContext   *link_context = NULL;

/* commands for the I/O loop */
static GMutex  *link_cmd_queue_lock = NULL;
static GList   *link_cmd_queue = NULL;

static int link_wakeup_fds[2] = { -1, -1 };
#define LINK_WAKEUP_POLL  link_wakeup_fds [0]
#define LINK_WAKEUP_WRITE link_wakeup_fds [1]
static GSource *link_main_source = NULL;
static GThread *link_io_thread = NULL;

#ifdef LINK_SSL_SUPPORT
SSL_METHOD *link_ssl_method;
SSL_CTX    *link_ssl_ctx;
#endif

/**
 * link_get_threaded:
 * 
 *   This routine returns TRUE if threading is enabled for
 * the ORB.
 **/
gboolean
link_get_threaded (void)
{
	return link_threaded;
}

static void
link_dispatch_command (gpointer data)
{
	LinkCommand *cmd = data;
	switch (cmd->type) {
	case LINK_COMMAND_SET_CONDITION:
		link_connection_exec_set_condition (data);
		break;
	case LINK_COMMAND_DISCONNECT:
		link_connection_exec_disconnect (data);
		break;
	default:
		g_error ("Unimplemented (%d)", cmd->type);
		break;
	}
}

static gboolean
link_mainloop_handle_input (GIOChannel   *source,
			    GIOCondition  condition,
			    gpointer      data)
{
	char c;
	GList *l, *queue;

	g_mutex_lock (link_cmd_queue_lock);
	{
		/* FIXME: read a big, non-blocking slurp here */
		read (LINK_WAKEUP_POLL, &c, sizeof (c));

		queue = link_cmd_queue;
		link_cmd_queue = NULL;
	}
	g_mutex_unlock (link_cmd_queue_lock);

	for (l = queue; l; l = l->next)
		link_dispatch_command (l->data);

	g_list_free (queue);

	return TRUE;
}

void
link_exec_command (LinkCommand *cmd)
{
	char c = 'A'; /* magic */
	int  res;

	if (!link_threaded) {
		link_dispatch_command (cmd);
		return;
	}

	LINK_MUTEX_LOCK (link_cmd_queue_lock);

	if (LINK_WAKEUP_WRITE == -1) { /* shutdown main loop */
		link_dispatch_command (cmd);
		LINK_MUTEX_UNLOCK (link_cmd_queue_lock);
		return;
	}

	/* FIXME: if (link_cmd_queue) - no need to wake mainloop */
	link_cmd_queue = g_list_append (link_cmd_queue, cmd);

	while ((res = write (LINK_WAKEUP_WRITE, &c, sizeof (c))) < 0  &&
	       (errno == EAGAIN || errno == EINTR));

	LINK_MUTEX_UNLOCK (link_cmd_queue_lock);
	if (res < 0)
		g_error ("Failed to write to linc wakeup socket %d 0x%x(%d) (%d)",
			 res, errno, errno, LINK_WAKEUP_WRITE);
}

static gpointer
link_io_thread_fn (gpointer data)
{
	link_main_loop_run ();

	/* FIXME: need to be able to quit without waiting ... */

	/* Asked to quit - so ...
	 * a) stop accepting inputs [ kill servers ]
	 * b) flush outgoing queued data etc. (oneways)
	 * c) unref all leakable resources.
	 */

	/* A tad of shutdown */
	LINK_MUTEX_LOCK (link_cmd_queue_lock);
	if (LINK_WAKEUP_WRITE >= 0) {
		close (LINK_WAKEUP_WRITE);
		close (LINK_WAKEUP_POLL);
		LINK_WAKEUP_WRITE = -1;
		LINK_WAKEUP_POLL = -1;
	}
	LINK_MUTEX_UNLOCK (link_cmd_queue_lock);

	if (link_main_source) {
		g_source_destroy (link_main_source);
		g_source_unref (link_main_source);
		link_main_source = NULL;
	}

	return NULL;
}

/**
 * link_init:
 * @init_threads: if we want threading enabled.
 * 
 * Initialize linc.
 **/
void
link_init (gboolean init_threads)
{
	if ((init_threads || link_threaded) &&
	    !g_thread_supported ())
		g_thread_init (NULL);

	if (!link_threaded && init_threads) {
		link_threaded = TRUE;
		_link_connection_thread_init (TRUE);
	}

	g_type_init ();

	/*
	 * Linc's raison d'etre is for ORBit2 and Bonobo
	 *
	 * In Bonobo, components and containers must not crash if the
	 * remote end crashes.  If a remote server crashes and then we
	 * try to make a CORBA call on it, we may get a SIGPIPE.  So,
	 * for lack of a better solution, we ignore SIGPIPE here.  This
	 * is open for reconsideration in the future.
	 *
	 * When SIGPIPE is ignored, write() calls which would
	 * ordinarily trigger a signal will instead return -1 and set
	 * errno to EPIPE.  So linc will be able to catch these
	 * errors instead of letting them kill the component.
	 *
	 * Possibilities are the MSG_PEEK trick, where you test if the
	 * connection is dead right before doing the writev().  That
	 * approach has two problems:
	 *
	 *   1. There is the possibility of a race condition, where
	 *      the remote end calls right after the test, and right
	 *      before the writev().
	 * 
	 *   2. An extra system call per write might be regarded by
	 *      some as a performance hit.
	 *
	 * Another possibility is to surround the call to writev() in
	 * link_connection_writev (linc-connection.c) with something like
	 * this:
	 *
	 *		link_ignore_sigpipe = 1;
	 *
	 *		result = writev ( ... );
	 *
	 *		link_ignore_sigpipe = 0;
	 *
	 * The SIGPIPE signal handler will check the global
	 * link_ignore_sigpipe variable and ignore the signal if it
	 * is 1.  If it is 0, it can proxy to the user's original
	 * signal handler.  This is a real possibility.
	 */
	signal (SIGPIPE, SIG_IGN);
	
	link_context = g_main_context_new ();
	link_loop    = g_main_loop_new (link_context, TRUE);
	
#ifdef LINK_SSL_SUPPORT
	SSLeay_add_ssl_algorithms ();
	link_ssl_method = SSLv23_method ();
	link_ssl_ctx = SSL_CTX_new (link_ssl_method);
#endif

	link_cmd_queue_lock  = link_mutex_new ();

	if (init_threads) {
		GError *error = NULL;

		if (pipe (link_wakeup_fds) < 0) /* cf. g_main_context_init_pipe */
			g_error ("Can't create CORBA main-thread wakeup pipe");

		link_main_source = link_source_create_watch
			(link_context, LINK_WAKEUP_POLL,
			 NULL, (G_IO_IN | G_IO_PRI),
			 link_mainloop_handle_input, NULL);

		link_io_thread = g_thread_create_full
			(link_io_thread_fn, NULL, 0, TRUE, FALSE,
			 G_THREAD_PRIORITY_NORMAL, &error);

		if (!link_io_thread || error)
			g_error ("Failed to create linc worker thread");
	}
}

/**
 * link_main_iteration:
 * @block_for_reply: whether we should wait for a reply
 * 
 * This routine iterates the linc mainloop, which has
 * only the linc sources registered against it.
 **/
void
link_main_iteration (gboolean block_for_reply)
{
	g_main_context_iteration (
		link_context, block_for_reply);
}

/**
 * link_main_pending:
 * 
 * determines if the linc mainloop has any pending work to process.
 * 
 * Return value: TRUE if the linc mainloop has any pending work to process.
 **/
gboolean
link_main_pending (void)
{
	return g_main_context_pending (link_context);
}

/**
 * link_main_loop_run:
 * 
 * Runs the linc mainloop; blocking until the loop is exited.
 **/
void
link_main_loop_run (void)
{
	g_main_loop_run (link_loop);
}

/**
 * link_mutex_new:
 * 
 * Creates a mutes, iff threads are supported, initialized and
 * link_set_threaded has been called.
 * 
 * Return value: a new GMutex, or NULL if one is not required.
 **/
GMutex *
link_mutex_new (void)
{
	link_mutex_new_called = TRUE;

#ifdef G_THREADS_ENABLED
	if (link_threaded && g_thread_supported ())
		return g_mutex_new ();
#endif

	return NULL;
}

/**
 * link_main_idle_add:
 * @function: method to call at idle
 * @data: user data.
 * 
 * Add an idle handler to the linc mainloop.
 * 
 * Return value: id of handler
 **/
guint 
link_main_idle_add (GSourceFunc    function,
		    gpointer       data)
{
	guint id;
	GSource *source;
  
	g_return_val_if_fail (function != NULL, 0);

	source = g_idle_source_new ();

	g_source_set_callback (source, function, data, NULL);
	id = g_source_attach (source, link_context);
	g_source_unref (source);

	return id;
}

gboolean
link_in_io_thread (void)
{
	return (!link_io_thread ||
		g_thread_self() == link_io_thread);
}

GMainLoop *
link_main_get_loop (void)
{
	return link_loop;
}

GMainContext *
link_main_get_context (void)
{
	return link_context;
}

gboolean
link_mutex_is_locked (GMutex *lock)
{
	gboolean result = TRUE;

	if (lock && g_mutex_trylock (lock)) {
		result = FALSE;
		g_mutex_unlock (lock);
	}

	return result;
}

void
link_shutdown (void)
{
	if (link_loop) /* break into the linc loop */
		g_main_loop_quit (link_loop);

	if (link_io_thread) {
		g_thread_join (link_io_thread);
		link_io_thread = NULL;
	}
}
