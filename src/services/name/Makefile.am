libORBitservicesincludedir = $(includedir)/ORBitservices

lib_LTLIBRARIES=libORBitCosNaming-2.la

bin_PROGRAMS = name-client orbit-name-server

lib_LIBRARIES = libname-server.a

libORBitCosNaming_2_la_SOURCES = 						\
	CosNaming-common.c 						\
	CosNaming-stubs.c

libORBitservicesinclude_HEADERS = 					\
	CosNaming.h							\
	CosNaming_impl.h						\
	CosNaming.idl

libORBitCosNaming_2_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

INCLUDES = -I. \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	 $(GLIB_CFLAGS) \
	$(LINC_CFLAGS)

CosNaming_built = 							\
	CosNaming.h 							\
	CosNaming-common.c 						\
	CosNaming-stubs.c 						\
	CosNaming-skels.c

IDL_FLAGS=--showcpperrors
IDL_FILES=CosNaming.idl
include $(top_srcdir)/Makefile.shared

CLEANFILES=my_cosnaming_built $(BUILT_SOURCES)

DEPS = 	$(top_builddir)/src/orb/libORBit-2.la			\
	libORBitCosNaming-2.la

LDADDS = \
	$(top_builddir)/src/orb/libORBit-2.la			\
	libORBitCosNaming-2.la					\
	@GLIB_LIBS@ 						\
	-lm

$(srcdir)/name-client.c $(srcdir)/orbit-name-server.c \
$(srcdir)/boot.c: CosNaming.h

name_client_SOURCES = name-client.c name-support.c name-support.h
name_client_LDFLAGS = $(FLAGS)
name_client_DEPENDENCIES = $(DEPS) CosNaming.h
name_client_LDADD = $(LDADDS)

libname_server_a_SOURCES = orbit-name-server.c CosNaming-skels.c \
			   name-support.c name-support.h
libname_server_a_DEPENDENCIES = $(DEPS) CosNaming.h

orbit_name_server_SOURCES = boot.c
orbit_name_server_LDFLAGS = $(FLAGS)
orbit_name_server_DEPENDENCIES = $(DEPS) CosNaming.h
orbit_name_server_LDADD = libname-server.a $(LDADDS)

EXTRA_DIST = CosNaming.idl

BUILT_SOURCES = $(CosNaming_built)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
