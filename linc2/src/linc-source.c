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
	LincUnixWatch *watch = (LincUnixWatch *)source;

	return watch->pollfd.revents & watch->condition;
}

static gboolean
link_source_dispatch (GSource    *source,
		      GSourceFunc callback,
		      gpointer    user_data)

{
	GIOFunc    func;
	LincUnixWatch *watch = (LincUnixWatch *) source;

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
	LincUnixWatch *watch = (LincUnixWatch *) source;

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
	LincUnixWatch *watch;

	source = g_source_new (&link_source_watch_funcs,
			       sizeof (LincUnixWatch));
	watch = (LincUnixWatch *) source;

	watch->pollfd.fd = fd;
	watch->channel   = opt_channel;

	link_source_set_condition (source, condition);

	g_source_set_can_recurse (source, TRUE);
	g_source_add_poll (source, &watch->pollfd);

	g_source_set_callback (source, (GSourceFunc) func,
			       user_data, NULL);
	g_source_attach (source, context);

	return source;
}

LincWatch *
link_io_add_watch_fd (int          fd,
		      GIOCondition condition,
		      GIOFunc      func,
		      gpointer     user_data)
{
	LincWatch *w;

	w = g_new0 (LincWatch, 1);

	/* Linc loop */
	w->link_source = link_source_create_watch (
		link_main_get_context (), fd, NULL,
		condition, func, user_data);

	if (!link_get_threaded ()) /* Main loop too */
		w->main_source = link_source_create_watch (
			NULL, fd, NULL, condition, func, user_data);
	else
		w->main_source = NULL;

	return w;
}

void
link_io_remove_watch (LincWatch *w)
{
	if (w) {
		link_source_set_condition (w->main_source, 0);
		link_source_set_condition (w->link_source, 0);

		if (w->main_source) {
			g_source_destroy (w->main_source);
			g_source_unref   (w->main_source);
		} /* else - threaded */
		
		g_source_destroy (w->link_source);
		g_source_unref   (w->link_source);

		g_free (w);
	}
}

void
link_watch_set_condition (LincWatch   *w,
			  GIOCondition condition)
{
	if (w) {
		link_source_set_condition (
			w->link_source, condition);

		link_source_set_condition (
			w->main_source, condition);
	}
}

