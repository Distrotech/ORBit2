#ifndef GIOP_H
#define GIOP_H 1

#include <linc/linc.h>
#define ORBIT_SSL_SUPPORT LINC_SSL_SUPPORT

#include <orbit/GIOP/giop-types.h>
#include <orbit/GIOP/giop-send-buffer.h>
#include <orbit/GIOP/giop-recv-buffer.h>
#include <orbit/GIOP/giop-connection.h>
#include <orbit/GIOP/giop-server.h>
#include <orbit/GIOP/giop-endian.h>

#ifdef ORBIT2_INTERNAL_API

void giop_init (void);

#endif /* ORBIT2_INTERNAL_API */

#endif
