SUBDIRS=include orb-core util GIOP poa dynamic

lib_LTLIBRARIES=libORBit.la

libORBit_la_SOURCES=dummy.c

libORBit_la_LDFLAGS=-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE) \
	$(LINC_LIBS) $(GLIB_LIBS)

libORBit_la_LIBADD= \
	util/liborb-util.la \
	GIOP/libGIOP.la \
	orb-core/liborb-core.la \
	poa/liborb-poa.la \
	dynamic/liborb-dynamic.la
