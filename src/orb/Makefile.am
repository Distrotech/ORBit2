SUBDIRS=util IIOP orb-core poa dynamic

lib_LTLIBRARIES=libORBit.la

libORBit_la_SOURCES=dummy.c

libORBit_la_LDFLAGS=-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) -release $(LT_RELEASE)

libORBit_la_LIBADD= \
	util/liborb-util.la \
	IIOP/libIIOP.la \
	orb-core/liborb-core.la \
	poa/liborb-poa.la \
	dynamic/liborb-dynamic.la
