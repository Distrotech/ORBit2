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
 * linc_get_threaded:
 * 
 *   This routine returns TRUE if threading is enabled for
 * the ORB.
 **/
gboolean
linc_get_threaded (void)
{
	return linc_threaded;
}

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
	g_warning ("linc_set_threaded is deprecated");

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

/**
 * linc_main_idle_add:
 * @function: method to call at idle
 * @data: user data.
 * 
 * Add an idle handler to the linc mainloop.
 * 
 * Return value: id of handler
 **/
guint 
linc_main_idle_add (GSourceFunc    function,
		    gpointer       data)
{
	guint id;
	GSource *source;
  
	g_return_val_if_fail (function != NULL, 0);

	source = g_idle_source_new ();

	g_source_set_callback (source, function, data, NULL);
	id = g_source_attach (source, linc_context);
	g_source_unref (source);

	return id;
}

gpointer
linc_object_ref (gpointer object)
{
	gpointer ret;

	LINC_MUTEX_LOCK   (linc_lifecycle_mutex);

	ret = g_object_ref (object);

	LINC_MUTEX_UNLOCK (linc_lifecycle_mutex);

	return ret;
}

static inline gboolean
linc_fast_unref (GObject *object)
{
	gboolean last_ref;

	LINC_MUTEX_LOCK   (linc_lifecycle_mutex);

	if (!(last_ref = (object->ref_count == 1)))
		g_object_unref (object);

	LINC_MUTEX_UNLOCK (linc_lifecycle_mutex);

	return last_ref;
}


static int
linc_idle_unref (gpointer object)
{
	if (linc_fast_unref (object))
		g_object_unref (object);
	return FALSE;
}

gboolean
linc_in_io_thread (void)
{
	gboolean result;

	/* FIXME: we can do a lot better here */
	if ((result = g_main_context_acquire (linc_context)))
		g_main_context_release (linc_context);

	return result;
}

void
linc_object_unref (gpointer object)
{
	if (linc_fast_unref (object)) {
		/* final unref outside the guard */
		if (linc_lifecycle_mutex) {
			if (g_main_context_acquire (linc_context)) {
				/* linc thread */
				g_object_unref (object);
				g_main_context_release (linc_context);
			} else {
				/* push to main linc thread */
				linc_main_idle_add (
					linc_idle_unref,
					object);
			}
		} else {
			/* only 1 thread anyway */
			g_object_unref (object);
		}
	}
}

GMainLoop *
linc_main_get_loop (void)
{
	return linc_loop;
}

GMainContext *
linc_main_get_context (void)
{
	return linc_context;
}
