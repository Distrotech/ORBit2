bin_PROGRAMS=orbit-idl

includedir=$(prefix)/include
orbitlibdir=$(libdir)/orbit-idl

INCLUDES=-I$(top_builddir) -I$(top_builddir)/include \
	 -I$(top_srcdir) -I$(top_srcdir)/include \
	@GLIB_CFLAGS@ \
	-DORBITLIBDIR="\"$(orbitlibdir)\"" \
	-DDEBUG -DVERSION=\"$(ORBIT_VERSION)\"

orbit_idl_DEPENDENCIES = $(top_builddir)/libIDL/libIDL-2.la
orbit_idl_LDADD = \
	$(top_builddir)/libIDL/libIDL-2.la \
	@GLIB_LIBS@ \
	-lpopt -lm

orbit_idl_SOURCES=orbit-idl-main.c \
		   orbit-idl-driver.c \
		   orbit-idl-backends.c \
		   orbit-idl-passes.c \
		   orbit-idl-genmarshal.c \
		   orbit-idl-utils.c \
		   orbit-idl2.h \
		   orbit-idl3-types.h \
        orbit-idl-c-backend.h \
        orbit-idl-c-backend.c \
        orbit-idl-c-stubs.c \
        orbit-idl-c-skels.c \
        orbit-idl-c-headers.c \
        orbit-idl-c-common.c \
	orbit-idl-c-imodule.c \
        orbit-idl-c-skelimpl.c \
        orbit-idl-c-utils.c \
        orbit-idl-c-typecode.c \
        orbit-idl-c-marshal.c \
        orbit-idl-c-demarshal.c \
        orbit-idl-c-deps.c \
	orbit-idl-marshal.c \
	orbit-idl-marshal.h

include_DATA=orbit-idl2.h

$(top_builddir)/libIDL/libIDL-2.la:
	$(MAKE) -C $(top_builddir)/libIDL libIDL-2.la
