SUBDIRS=include orb-core util GIOP poa goa dynamic

lib_LTLIBRARIES=libORBit-2.la

INCLUDES =					\
	-I.					\
	-I$(srcdir)				\
	-I$(top_srcdir)/include			\
	-I$(top_builddir)/include		\
	-DORBIT_VERSION=\"@ORBIT_VERSION@\"	\
	-DORBIT2_INTERNAL_API			\
	$(WARN_CFLAGS)				\
	$(ORBIT_DEBUG_CFLAGS)			\
	$(ORBIT_CFLAGS)

libORBit_2_la_SOURCES=orbit-init.c orbit-init.h

libORBit_2_la_LDFLAGS =               \
	-version-info $(LT_VERSION)   \
	$(ORBIT_LIBS)                 \
	-lpopt

libORBit_2_la_LIBADD= \
	util/liborb-util.la \
	GIOP/libGIOP.la \
	orb-core/liborb-core.la \
	poa/liborb-poa.la \
	goa/liborb-goa.la \
	dynamic/liborb-dynamic.la

inst-lib: all-recursive install-libLTLIBRARIES
