#ifndef LINC_TYPES_H
#define LINC_TYPES_H 1

#include <glib.h>
#include <glib-object.h>

typedef enum {
  LINC_CONNECTION_SSL
} LINCConnectionOptions;

#ifndef O_MUTEX_DEFINE
#define O_MUTEX_DEFINE(x)
#define O_MUTEX_INIT(x)
#define O_MUTEX_LOCK(x)
#define O_MUTEX_UNLOCK(x)
#define O_MUTEX_DESTROY(x)
#endif

#ifndef orbit_alloca
#define orbit_alloca alloca
#endif

#endif
