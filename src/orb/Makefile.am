SUBDIRS=include orb-core util GIOP poa dynamic

lib_LTLIBRARIES=libORBit-2.la

INCLUDES=-I. -I$(srcdir) -I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	@GLIB_CFLAGS@ \
	@LINC_CFLAGS@

libORBit_2_la_SOURCES=orbit-init.c orbit-init.h

libORBit_2_la_LDFLAGS=-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-release $(LT_RELEASE) \
	$(LINC_LIBS) $(GLIB_LIBS) \
	-lpopt

libORBit_2_la_LIBADD= \
	util/liborb-util.la \
	GIOP/libGIOP.la \
	orb-core/liborb-core.la \
	poa/liborb-poa.la \
	dynamic/liborb-dynamic.la

inst-lib: all-recursive install-libLTLIBRARIES
