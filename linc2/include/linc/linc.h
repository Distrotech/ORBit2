#ifndef LINC_H
#define LINC_H 1

#include <linc/linc-config.h>
#include <linc/linc-types.h>
#include <linc/linc-threads.h>
#include <linc/linc-protocol.h>
#include <linc/linc-connection.h>
#include <linc/linc-server.h>

extern GMainLoop *linc_loop;

void       linc_init            (void);

LincWatch *linc_io_add_watch    (GIOChannel    *channel,
				 GIOCondition   condition,
				 GIOFunc        func,
				 gpointer       user_data);
void       linc_io_remove_watch (LincWatch     *watch);
void       linc_main_iteration  (gboolean       block_for_reply);
gboolean   linc_main_pending    (void);
void       linc_main_loop_run   (void);

#endif
