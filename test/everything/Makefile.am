noinst_PROGRAMS = client server

EVERYTHING_BUILT = \
	everything.h        \
	everything-skels.c  \
	everything-stubs.c  \
	everything-common.c

EVERYTHING_IDLOUT =          \
	$(EVERYTHING_BUILT)  \
	everything-imodule.c

INCLUDES =                                 \
	-I$(top_srcdir)/include            \
	-I$(top_builddir)/include          \
	-I$(top_builddir)/src              \
	-I$(top_builddir)/src/ORBitutil    \
	-I$(top_srcdir)/src/orb            \
	-I$(top_srcdir)/src                \
	$(ORBIT_CFLAGS)

#	-DORBIT_STUB_DEBUG_LOCAL

LDADD = $(top_builddir)/src/orb/libORBit-2.la		\
	$(ORBIT_LIBS)

#  -lefence

client_SOURCES=${EVERYTHING_BUILT} client.c everything.idl constants.h
client_DEPENDENCES=${EVERYTHING_BUILT} $(included_src)
client_LDADD = $(LDADD)
client_LDFLAGS = -static -module

included_src = \
	anyServer.c	 \
	arrayServer.c    \
	basicServer.c    \
	contextServer.c  \
	deadReference.c  \
	sequenceServer.c \
	structServer.c   \
	unionServer.c

orbittypelibdir = $(libdir)/orbit-2.0
orbittypelib_LTLIBRARIES = Everything_module.la

Everything_module_la_LDFLAGS = \
	-export-dynamic -module
Everything_module_la_SOURCES = \
	everything-imodule.c

server_SOURCES=server.c ${EVERYTHING_BUILT} everything.idl constants.h
server_DEPENDENCES=${EVERYTHING_BUILT} $(included_src)
server_LDFLAGS = -static -module

IDL_FLAGS= --showcpperrors --add-imodule
IDL_FILES= everything.idl
include $(top_srcdir)/Makefile.shared

BUILT_SOURCES = ${EVERYTHING_BUILT} everything-imodule.c
CLEANFILES = ${BUILT_SOURCES} iorfile

TESTS = $(srcdir)/test.sh

BUILT_SOURCES = $(EVERYTHING_IDLOUT)
CLEANFILES = $(BUILT_SOURCES)
EXTRA_DIST = $(IDL_FILES) test.sh $(included_src)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
