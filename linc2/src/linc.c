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
#include <signal.h>
#include "linc-debug.h"
#include "linc-private.h"

static gboolean linc_threaded = FALSE;
static gboolean linc_mutex_new_called = FALSE;
GMainLoop      *linc_loop = NULL;
GMainContext   *linc_context = NULL;
static GMutex  *linc_lifecycle_mutex = NULL;

#ifdef LINC_SSL_SUPPORT
SSL_METHOD *linc_ssl_method;
SSL_CTX    *linc_ssl_ctx;
#endif

/**
 * linc_set_threaded:
 * @threaded: whether to do locking
 * 
 *   This routine turns threading on or off for the whole
 * ORB, it should be called (TRUE) if threading is desired
 * before any of the ORB initialization occurs.
 **/
void
linc_set_threaded (gboolean threaded)
{
	if (linc_mutex_new_called)
		g_error ("You need to set this before using the ORB");
	linc_threaded = threaded;
}

/**
 * linc_init:
 * @init_threads: if we want threading enabled.
 * 
 * Initialize linc.
 **/
void
linc_init (gboolean init_threads)
{
	if ((init_threads || linc_threaded) &&
	    !g_thread_supported ())
		g_thread_init (NULL);

	if (!linc_threaded && init_threads)
		linc_threaded = TRUE;

	g_type_init_with_debug_flags (G_TYPE_DEBUG_OBJECTS);

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
	 * linc_connection_writev (linc-connection.c) with something like
	 * this:
	 *
	 *		linc_ignore_sigpipe = 1;
	 *
	 *		result = writev ( ... );
	 *
	 *		linc_ignore_sigpipe = 0;
	 *
	 * The SIGPIPE signal handler will check the global
	 * linc_ignore_sigpipe variable and ignore the signal if it
	 * is 1.  If it is 0, it can proxy to the user's original
	 * signal handler.  This is a real possibility.
	 */
	signal (SIGPIPE, SIG_IGN);
	
	linc_context = g_main_context_new ();
	linc_loop    = g_main_loop_new (linc_context, TRUE);
	
#ifdef LINC_SSL_SUPPORT
	SSLeay_add_ssl_algorithms ();
	linc_ssl_method = SSLv23_method ();
	linc_ssl_ctx = SSL_CTX_new (linc_ssl_method);
#endif

	linc_lifecycle_mutex = linc_mutex_new ();
}

struct _LincWatch {
	guint main_id;
	guint linc_id;
};

#ifdef WATCH_DEBUG
static GSList *watches = NULL;
#endif

/**
 * linc_io_add_watch:
 * @channel: the GIOChannel to watch
 * @condition: the condition mask to watch for
 * @func: the function to invoke when a condition is met
 * @user_data: a user data closure
 * 
 * This routine creates a watch on an IO channel that operates both in
 * the standard glib mainloop, but also in the 'linc' mainloop so we
 * can iterate that without causing re-enterancy.
 * 
 * Return value: a pointer identifying the watch.
 **/
LincWatch *
linc_io_add_watch (GIOChannel    *channel,
		   GIOCondition   condition,
		   GIOFunc        func,
		   gpointer       user_data)
{
	GSource  *source;
	LincWatch *w;
  
	g_return_val_if_fail (channel != NULL, 0);

	w = g_new (LincWatch, 1);

	/* Linc loop */
	source = g_io_create_watch (channel, condition);
	g_source_set_can_recurse (source, TRUE);
	g_source_set_callback (source, (GSourceFunc)func, user_data, NULL);
	w->linc_id = g_source_attach (source, linc_context);
	g_source_unref (source);

	/* Main loop */
	source = g_io_create_watch (channel, condition);
	g_source_set_can_recurse (source, TRUE);
	g_source_set_callback (source, (GSourceFunc)func, user_data, NULL);
	w->main_id = g_source_attach (source, NULL);
	g_source_unref (source);

#ifdef WATCH_DEBUG
	source = g_main_context_find_source_by_id (
		NULL, w->main_id);
	g_assert (source != NULL);
	g_assert (source->poll_fds != NULL);

	source = g_main_context_find_source_by_id (
		linc_context, w->linc_id);
	g_assert (source != NULL);
	g_assert (source->poll_fds != NULL);

	watches = g_slist_prepend (watches, w);
#endif
	
	return w;
}

/**
 * linc_io_remove_watch:
 * @watch: the handle of a watch on a GIOChannel
 * 
 * This removes a watch by it's handle in @w
 **/
void
linc_io_remove_watch (LincWatch *w)
{
	if (w) {
		GSource *source;

#ifdef WATCH_DEBUG
		watches = g_slist_remove (watches, w);
#endif

		/* If these warnings fire, we're prolly returning
		   TRUE from the source's handler somewhere in error */
		   
		source = g_main_context_find_source_by_id (
			NULL, w->main_id);
		if (source)
			g_source_destroy (source);
		else
			g_warning ("Missing source on main context");

		source = g_main_context_find_source_by_id (
			linc_context, w->linc_id);
		if (source)
			g_source_destroy (source);
		else
			g_warning ("Missing source on linc context");

		g_free (w);
	}
}

/**
 * linc_main_iteration:
 * @block_for_reply: whether we should wait for a reply
 * 
 * This routine iterates the linc mainloop, which has
 * only the linc sources registered against it.
 **/
void
linc_main_iteration (gboolean block_for_reply)
{
#ifdef WATCH_DEBUG
	GSList *l;

	g_assert (watches != NULL);

	for (l = watches; l; l = l->next) {
		LincWatch *w = l->data;
		GSource *source = g_main_context_find_source_by_id (
			linc_context, w->linc_id);
		GPollFD *fd;

		g_warning ("source %p", source);

		fd = source->poll_fds->data;
		g_warning ("pollrec fd %d, (0x%x, 0x%x)",
			   fd->fd, fd->events, fd->revents);
	}
#endif

	g_main_context_iteration (
		linc_context, block_for_reply);
}

/**
 * linc_main_pending:
 * 
 * determines if the linc mainloop has any pending work to process.
 * 
 * Return value: TRUE if the linc mainloop has any pending work to process.
 **/
gboolean
linc_main_pending (void)
{
	return g_main_context_pending (linc_context);
}

/**
 * linc_main_loop_run:
 * 
 * Runs the linc mainloop; blocking until the loop is exited.
 **/
void
linc_main_loop_run (void)
{
	g_main_loop_run (linc_loop);
}

/**
 * linc_mutex_new:
 * 
 * Creates a mutes, iff threads are supported, initialized and
 * linc_set_threaded has been called.
 * 
 * Return value: a new GMutex, or NULL if one is not required.
 **/
GMutex *
linc_mutex_new (void)
{
	linc_mutex_new_called = TRUE;

#ifdef G_THREADS_ENABLED
	if (linc_threaded && g_thread_supported ())
		return g_mutex_new ();
#endif

	return NULL;
}

GMutex *
linc_object_get_mutex (void)
{
	return linc_lifecycle_mutex;
}

gpointer
linc_object_ref (GObject *object)
{
	gpointer ret;

	LINC_MUTEX_LOCK   (linc_lifecycle_mutex);

	ret = g_object_ref (object);

	LINC_MUTEX_UNLOCK (linc_lifecycle_mutex);

	return ret;
}

void
linc_object_unref (GObject *object)
{
	LINC_MUTEX_LOCK   (linc_lifecycle_mutex);

	g_object_unref (object);

	/* FIXME: we need to do our own ref counting here
	 * so finalization doesn't happen whilst holding
	 * this lock ! can't use these methods until then */

	LINC_MUTEX_UNLOCK (linc_lifecycle_mutex);
}

GMainLoop *
linc_main_get_loop (void)
{
	return linc_loop;
}
