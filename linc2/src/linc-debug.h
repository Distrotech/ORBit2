#ifndef _LINK_DEBUG_H
#define _LINK_DEBUG_H

/*
 * Enables debug on the Unix socket / connection
 */
#undef CONNECTION_DEBUG

#ifndef CONNECTION_DEBUG
#ifdef G_HAVE_ISO_VARARGS
#  define d_printf(...)
#elif defined(G_HAVE_GNUC_VARARGS)
#  define d_printf(args...)
#else
static inline void d_printf (const char *format, ...)
{
}
#endif
#  define STATE_NAME(s) ""
#else
#  include <stdio.h>
#  define d_printf(format...) fprintf (stderr, format)
#  define STATE_NAME(s) (((s) == LINK_CONNECTED) ? "Connected" : \
			 ((s) == LINK_CONNECTING) ? "Connecting" : \
			 ((s) == LINK_DISCONNECTED) ? "Disconnected" : \
			 "Invalid state")
#endif

#endif /* _LINK_DEBUG_H */
