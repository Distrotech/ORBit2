
bin_PROGRAMS=orbit-idl t

INCLUDES=-I$(top_builddir)/include -I$(top_srcdir)/include \
	$(GLIB_CFLAGS) \
	-DORBIT_SERIAL=@ORBIT_SERIAL@

orbit_idl_DEPENDENCIES = $(top_builddir)/libIDL/libIDL.la

orbit_idl_LDADD = \
	$(top_builddir)/libIDL/libIDL.la \
	$(GLIB_LIBS) $(POPT_LIBS) -lm

orbit_idl_SOURCES = \
	orbit-idl-main.c \
	orbit-idl-driver.c \
	orbit-idl-utils.c \
	orbit-idl-c-backend.h \
	orbit-idl-c-backend.c \
	orbit-idl-c-common.c \
	orbit-idl-c-headers.c \
	orbit-idl-c-skelimpl.c \
	orbit-idl-c-skels.c \
	orbit-idl-c-stubs.c \
	orbit-idl-c-typecode.c \
	orbit-idl-c-utils.c \
	orbit-idl-marshal.c \
	orbit-idl3.h \
	orbit-idl-marshal.h

t_SOURCES=t.c orbit-idl-utils.c
t_LDADD = \
	$(top_builddir)/libIDL/libIDL.la \
	$(GLIB_LIBS) $(POPT_LIBS) -lm
