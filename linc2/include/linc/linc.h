#ifndef LINC_H
#define LINC_H 1

#include <linc/linc-config.h>
#include <linc/linc-types.h>
#include <linc/linc-protocol.h>
#include <linc/linc-connection.h>
#include <linc/linc-server.h>

extern GMainLoop *linc_loop;
void linc_init(void);

#endif
