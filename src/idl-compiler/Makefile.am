bin_PROGRAMS=orbit-idl-2

orbitlibdir=$(libdir)/orbit-idl

INCLUDES =                                    \
	-I$(top_builddir)                     \
	-I$(top_builddir)/include             \
	 -I$(top_srcdir)                      \
	-I$(top_srcdir)/include               \
	-DORBITLIBDIR="\"$(orbitlibdir)\""    \
	-DVERSION=\"$(ORBIT_VERSION)\"        \
	-DORBIT2_INTERNAL_API                 \
	$(WARN_CFLAGS)                        \
	$(DISABLE_DEPRECATED_CFLAGS)          \
	$(ORBIT_IDL_CFLAGS)

orbit_idl_2_LDADD =                          \
	-lpopt -lm                           \
	$(ORBIT_IDL_LIBS)

orbit_idl_2_SOURCES=orbit-idl-main.c \
		   orbit-idl-driver.c \
		   orbit-idl-backends.c \
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
        orbit-idl-c-deps.c \
	orbit-idl2.h
