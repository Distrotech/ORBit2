libIIOPincludedir = $(includedir)/ORBit/IIOP

INCLUDES=-I$(top_srcdir)/include @GLIB_CFLAGS@

noinst_LTLIBRARIES=libIIOP.la

srcs=	giop-connection.c \
	iiop-connection.c \
	giop-msg-buffer.c \
	giop-recv-buffer.c \
	giop-send-buffer.c \
	IIOP-private.h

libIIOP_la_SOURCES=dummy.c

libIIOPinclude_HEADERS = \
	giop-connection.h \
	iiop-connection.h \
	giop-msg-buffer.h \
	giop-recv-buffer.h \
	giop-send-buffer.h \
	giop-types.h \
	giop-msg.h \
	IIOP.h 

libIIOP_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
