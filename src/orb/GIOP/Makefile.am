INCLUDES=-I$(top_srcdir)/include -I$(top_builddir)/include \
	$(GLIB_CFLAGS) \
	$(LINC_CFLAGS)

noinst_LTLIBRARIES=libGIOP.la

libGIOP_la_SOURCES=giop-connection.c \
	giop-server.c \
	giop.c \
	giop-private.h \
	giop-send-buffer.c \
	giop-recv-buffer.c \
	giop-endian.c

libGIOP_la_LDFLAGS = -version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)
