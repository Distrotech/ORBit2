#ifndef BASIC_TYPES_H
#define BASIC_TYPES_H 1

#include <glib.h>

typedef gint16 CORBA_short;
typedef gint32 CORBA_long;
typedef guint16 CORBA_unsigned_short;
typedef guint32 CORBA_unsigned_long;
typedef gfloat CORBA_float;
typedef gdouble CORBA_double;
typedef char CORBA_char;
typedef guchar CORBA_boolean;
typedef guchar CORBA_octet;
typedef gdouble CORBA_long_double;
typedef guint16 CORBA_wchar; /* I'm not sure what size a wchar is supposed to be */

typedef struct CORBA_Object_type *CORBA_Object;

#ifdef G_HAVE_GINT64
#define HAVE_CORBA_LONG_LONG
/* According to the spec, these two are optional. We support them if we can. */
typedef gint64 CORBA_long_long;
typedef guint64 CORBA_unsigned_long_long;
typedef CORBA_long_long GIOP_long_long;
typedef CORBA_unsigned_long_long GIOP_unsigned_long_long;
#else
#warning ""
#warning "You don't G_HAVE_GINT64 defined in glib."
#warning "Please make sure you don't have an old glibconfig.h lying around."
#warning ""
#endif

#endif
