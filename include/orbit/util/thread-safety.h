#ifndef THREAD_SAFETY_H
#define THREAD_SAFETY_H 1

#ifdef ORBIT_THREADSAFE

#include <pthread.h>

#define O_MUTEX_DEFINE(x) GMutex* x
#define O_MUTEX_INIT(x) (x) = g_mutex_new()
#define O_MUTEX_LOCK(x) g_mutex_lock(x)
#define O_MUTEX_UNLOCK(x) g_mutex_unlock(x)
#define O_MUTEX_DESTROY(x) g_mutex_free(x)

#else

#define O_MUTEX_DEFINE(x)
#define O_MUTEX_INIT(x)
#define O_MUTEX_LOCK(x)
#define O_MUTEX_UNLOCK(x)
#define O_MUTEX_DESTROY(x)

#endif

#endif /* THREAD_SAFETY_H */
