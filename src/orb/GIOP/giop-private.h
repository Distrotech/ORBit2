#ifndef GIOP_PRIVATE_H
#define GIOP_PRIVATE_H 1

#include "config.h"
#include <orbit/orbit-config.h>
#include <orbit/GIOP/giop.h>
#include <linc/linc.h>

void giop_send_buffer_init(void);
void giop_recv_buffer_init(void);
void giop_connection_list_init(void);

O_MUTEX_DEFINE_EXTERN(giop_queued_messages_lock);
extern GList *giop_queued_messages;

#endif
