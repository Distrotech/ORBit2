#ifndef GIOP_BASICS_H
#define GIOP_BASICS_H 1

#include <linc/linc.h>

#ifdef LINC_THREADSAFE
#define ORBIT_THREADSAFE 1
/* #define ORBIT_THREADED 1 */
#endif

typedef struct _GIOPRecvBuffer GIOPRecvBuffer;
typedef struct _GIOPSendBuffer GIOPSendBuffer;
typedef struct _GIOPConnection GIOPConnection;

typedef enum {
  GIOP_1_0,
  GIOP_1_1,
  GIOP_1_2,
  GIOP_LATEST = GIOP_1_2,
  GIOP_NUM_VERSIONS
} GIOPVersion;

#endif
