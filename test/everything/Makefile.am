noinst_PROGRAMS=client server

EVERYTHING_BUILT = \
	everything.h       \
	everything-stubs.c \
	everything-skels.c \
	everything-common.c

INCLUDES = \
	-I$(top_srcdir)/include		\
	-I$(top_builddir)/include		\
	-I$(top_builddir)/src		\
	-I$(top_builddir)/src/ORBitutil \
	-I$(top_srcdir)/src/orb		\
	-I$(top_srcdir)/src		\
	$(LINC_CFLAGS)			\
	${GLIB_CFLAGS}

LDADD = $(top_builddir)/src/orb/libORBit.la		\
	$(LINC_LIBS)					\
	${GLIB_LIBS}

#	$(top_builddir)/src/IIOP/libIIOP.la		\
#	$(top_builddir)/src/ORBitutil/libORBitutil.la	\

LDFLAGS = -static

client_SOURCES=client.c ${EVERYTHING_BUILT} everything.idl constants.h
client_DEPENDENCES=${EVERYTHING_BUILT} $(included_src)

included_src = \
	basicServer.c    \
	structServer.c   \
	sequenceServer.c \
	unionServer.c    \
	arrayServer.c    \
	anyServer.c

server_SOURCES=server.c ${EVERYTHING_BUILT} everything.idl constants.h
server_DEPENDENCES=${EVERYTHING_BUILT} $(included_src)

ORBIT_IDL = $(top_builddir)/src/idl-compiler/orbit-idl

${EVERYTHING_BUILT}: everything.idl $(ORBIT_IDL)
	$(ORBIT_IDL) $(srcdir)/everything.idl

CLEANFILES = ${EVERYTHING_BUILT} iorfile
BUILT_SOURCES = ${EVERYTHING_BUILT}

TESTS = $(srcdir)/test.sh

EXTRA_DIST = test.sh $(included_src)
