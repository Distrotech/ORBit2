#ifndef ORBIT_UTIL_H
#define ORBIT_UTIL_H 1

#include <glib.h>
#include <orbit/orbit-config.h>
#include <orbit/util/os-feature-alloca.h>

/* Align an address upward to a boundary, expressed as a number of bytes.
   E.g. align to an 8-byte boundary with argument of 8.  */

/*
 *   (this + boundary - 1)
 *          &
 *    ~(boundary - 1)
 */

#define ALIGN_ADDRESS(this, boundary) \
  ((gpointer)((( ((gulong)(this)) + (((gulong)(boundary)) -1)) & (~(((gulong)(boundary))-1)))))

#endif
