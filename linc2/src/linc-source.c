/*
 * linc-source.c: This file is part of the linc library.
 *
 * Authors:
 *    Owen Taylor   (owen@redhat.com)
 *    Michael Meeks (michael@ximian.com)
 *
 * Copyright 1998, 2001, Red Hat, Inc., Ximian, Inc.,
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <glib.h>
#include "linc-debug.h"
#include "linc-private.h"

static gboolean 
link_source_prepare (GSource *source,
		     gint    *timeout)
{
	*timeout = -1;

	return FALSE;
}

static gboolean 
link_source_check (GSource *source)
{
	LinkUnixWatch *watch = (LinkUnixWatch *)source;

	return watch->pollfd.revents & watch->condition;
}

static gboolean
link_source_dispatch (GSource    *source,
		      GSourceFunc callback,
		      gpointer    user_data)

{
	GIOFunc    func;
	LinkUnixWatch *watch = (LinkUnixWatch *) source;

	if (!callback)
		g_error ("No callback");
  
	func = (GIOFunc) callback;

	return (*func) (watch->channel,
			watch->pollfd.revents & watch->condition,
			user_data);
}

static void
link_source_finalize (GSource *source)
{
	d_printf ("Finalize source %p", source);
}

GSourceFuncs link_source_watch_funcs = {
	link_source_prepare,
	link_source_check,
	link_source_dispatch,
	link_source_finalize
};

/**
 * link_source_set_condition:
 * @source: a source created with #link_source_create_watch
 * @condition: a new condition.
 * 
 *     This sets a new IO condition on an existing
 * source very rapidly.
 **/
void
link_source_set_condition (GSource      *source,
			   GIOCondition  condition)
{
	LinkUnixWatch *watch = (LinkUnixWatch *) source;

	if (watch) {
		watch->pollfd.events = condition;
		watch->condition     = condition;
	}
}

/**
 * link_source_create_watch:
 * @context: context to add to (or NULL for default)
 * @fd: file descriptor to poll on
 * @opt_channel: channel, handed to the callback (can be NULL)
 * @condition: IO condition eg. G_IO_IN|G_IO_PRI
 * @func: callback when condition is met
 * @user_data: callback closure.
 * 
 * This adds a new source to the specified context.
 * 
 * Return value: the source handle so you can remove it later.
 **/
GSource *
link_source_create_watch (GMainContext *context,
			  int           fd,
			  GIOChannel   *opt_channel,
			  GIOCondition  condition,
			  GIOFunc       func,
			  gpointer      user_data)
{
	GSource       *source;
	LinkUnixWatch *watch;

	source = g_source_new (&link_source_watch_funcs,
			       sizeof (LinkUnixWatch));
	watch = (LinkUnixWatch *) source;

	watch->pollfd.fd = fd;
	watch->channel   = opt_channel;
	watch->condition = condition;
	watch->callback  = func;
	watch->user_data = user_data;

	link_source_set_condition (source, condition);

	g_source_set_can_recurse (source, TRUE);
	g_source_add_poll (source, &watch->pollfd);

	g_source_set_callback (source, (GSourceFunc) func,
			       user_data, NULL);
	g_source_attach (source, context);

	return source;
}

LinkWatch *
link_io_add_watch_fd (int          fd,
		      GIOCondition condition,
		      GIOFunc      func,
		      gpointer     user_data)
{
	LinkWatch *w;
	GMainContext *thread_ctx;

	w = g_new0 (LinkWatch, 1);

	if ((thread_ctx = link_thread_io_context ())) {
		/* Have a dedicated I/O worker thread */
		w->link_source = link_source_create_watch
			(thread_ctx, fd, NULL, condition, func, user_data);

	} else {
		/* Have an inferior and hook into the glib context */

		/* Link loop */
		w->link_source = link_source_create_watch
			(link_main_get_context (), fd, NULL,
			 condition, func, user_data);
		
		w->main_source = link_source_create_watch
			(NULL, fd, NULL, condition, func, user_data);
	}

	return w;
}

static void
link_watch_unlisten (LinkWatch *w)
{
	if (w->main_source) {
		link_source_set_condition (w->main_source, 0);
		g_source_destroy (w->main_source);
		g_source_unref   (w->main_source);
		w->main_source = NULL;
	}

	if (w->link_source) {
		link_source_set_condition (w->link_source, 0);
		g_source_destroy (w->link_source);
		g_source_unref   (w->link_source);	
		w->link_source = NULL;
	}
}

void
link_io_remove_watch (LinkWatch *w)
{
	if (!w)
		return;

	link_watch_unlisten (w);
	g_free (w);
}

void
link_watch_set_condition (LinkWatch   *w,
			  GIOCondition condition)
{
	if (w) {
		link_source_set_condition (
			w->link_source, condition);

		link_source_set_condition (
			w->main_source, condition);
	}
}

/*
 * Migrates the source to/from the main thread.
 */
void
link_watch_move_io (LinkWatch *w,
		    gboolean to_io_thread)
{
	LinkUnixWatch w_cpy;

	if (!w)
		return;

	g_assert (to_io_thread); /* FIXME */

	w_cpy = *(LinkUnixWatch *)w->link_source;

	link_watch_unlisten (w);

	w->link_source = link_source_create_watch
		(link_thread_io_context (),
		 w_cpy.pollfd.fd,
		 w_cpy.channel, w_cpy.condition,
		 w_cpy.callback, w_cpy.user_data);
}
