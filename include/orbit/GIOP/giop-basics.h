#ifndef GIOP_BASICS_H
#define GIOP_BASICS_H 1

#include <linc/linc.h>

#ifdef LINC_THREADSAFE
#define ORBIT_THREADSAFE 1
#endif

typedef struct _GIOPRecvBuffer GIOPRecvBuffer;
typedef struct _GIOPSendBuffer GIOPSendBuffer;
typedef struct _GIOPConnection GIOPConnection;

#endif
