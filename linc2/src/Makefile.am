lib_LTLIBRARIES=liblinc.la

INCLUDES =                         \
	-I$(top_builddir)/include  \
	-I$(top_srcdir)/include    \
	$(LINC_CFLAGS)             \
	$(SSL_CFLAGS)              \
	$(WARN_CFLAGS)             \
	-D_GNU_SOURCE

liblinc_la_SOURCES =		\
	linc.c			\
	linc-connection.c	\
	linc-debug.h		\
	linc-private.h		\
	linc-protocols.c	\
	linc-server.c		\
	linc-source.c

liblinc_la_LDFLAGS =                 \
	$(LINC_LIBS)                 \
	$(SSL_LIBS)                  \
	-version-info $(LT_VERSION)
