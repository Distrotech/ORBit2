lib_LTLIBRARIES=liblinc.la

INCLUDES=-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	$(GLIB_CFLAGS)

liblinc_la_SOURCES=\
	linc.c \
	linc-connection.c \
	linc-protocols.c \
	linc-server.c \
	linc-private.h

liblinc_la_LDFLAGS=$(GLIB_LIBS) $(SSL_LIBS)
