/*
 * linc.h: This file is part of the linc library.
 *
 * Authors:
 *    Elliot Lee     (sopwith@redhat.com)
 *    Michael Meeks  (michael@ximian.com)
 *    Mark McLouglin (mark@skynet.ie) & others
 *
 * Copyright 2001, Red Hat, Inc., Ximian, Inc.,
 *                 Sun Microsystems, Inc.
 */
#ifndef _LINK_H_
#define _LINK_H_

#include <linc/linc-config.h>
#include <linc/linc-types.h>
#include <linc/linc-protocol.h>
#include <linc/linc-connection.h>
#include <linc/linc-server.h>
#include <linc/linc-source.h>

G_BEGIN_DECLS

extern GMainLoop *link_loop;

void       link_init             (gboolean       init_threads);
void       link_shutdown         (void);

LincWatch *link_io_add_watch     (GIOChannel    *channel,
				  GIOCondition   condition,
				  GIOFunc        func,
				  gpointer       user_data);
void       link_io_remove_watch  (LincWatch     *watch);
void       link_main_iteration   (gboolean       block_for_reply);
gboolean   link_main_pending     (void);
void       link_main_loop_run    (void);
GMainLoop *link_main_get_loop    (void);
guint      link_main_idle_add    (GSourceFunc    function,
				  gpointer       data);

/* Deprecated bits ... */
void       link_set_threaded     (gboolean       threaded);
gpointer   link_object_ref       (gpointer object);
void       link_object_unref     (gpointer object);
GMutex    *link_object_get_mutex (void);

G_END_DECLS

#endif /* _LINK_H_ */
