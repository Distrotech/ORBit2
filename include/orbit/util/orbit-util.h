#ifndef ORBIT_UTIL_H
#define ORBIT_UTIL_H 1

#include <glib.h>
#include <orbit/orbit-config.h>
#include <orbit/util/basic_types.h>
#include <orbit/util/thread-safety.h>

/* Align a value upward to a boundary, expressed as a number of bytes.
   E.g. align to an 8-byte boundary with argument of 8.  */

/*
 *   (this + boundary - 1)
 *          &
 *    ~(boundary - 1)
 */

#define ALIGN_VALUE(this, boundary) \
  (( ((gulong)(this)) + (((gulong)(boundary)) -1)) & (~(((gulong)(boundary))-1)))

#define ALIGN_ADDRESS(this, boundary) \
  ((gpointer)ALIGN_ADDRESS(this, boundary))

gulong ORBit_wchar_strlen(CORBA_wchar *wstr);

#endif
