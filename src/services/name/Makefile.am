libORBitservicesincludedir = $(includedir)/ORBitservices

lib_LTLIBRARIES=libORBitCosNaming.la

bin_PROGRAMS = name-client orbit-name-server

lib_LIBRARIES = libname-server.a

libORBitCosNaming_la_SOURCES = 						\
	CosNaming-common.c 						\
	CosNaming-stubs.c

libORBitservicesinclude_HEADERS = 					\
	CosNaming.h							\
	CosNaming_impl.h						\
	CosNaming.idl

libORBitCosNaming_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)

INCLUDES = -I. \
	-I$(top_builddir)/include \
	-I$(top_srcdir)/include \
	 $(GLIB_CFLAGS) \
	$(LINC_CFLAGS)

IDL = $(top_builddir)/src/idl-compiler/orbit-idl

CosNaming_built = 							\
	CosNaming.h 							\
	CosNaming-common.c 						\
	CosNaming-stubs.c 						\
	CosNaming-skels.c

$(CosNaming_built): my_cosnaming_built
	@:

my_cosnaming_built: CosNaming.idl $(IDL)
	$(IDL) $(srcdir)/CosNaming.idl
	touch my_cosnaming_built

CLEANFILES=my_cosnaming_built

DEPS = 	$(top_builddir)/src/orb/libORBit.la			\
	libORBitCosNaming.la

LDADDS = \
	$(top_builddir)/src/orb/libORBit.la			\
	libORBitCosNaming.la					\
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
