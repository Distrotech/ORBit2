
bin_PROGRAMS=orbit-idl

INCLUDES=-I$(top_builddir)/include -I$(top_srcdir)/include \
	$(GLIB_CFLAGS)

orbit_idl_DEPENDENCIES = $(top_builddir)/libIDL/libIDL.la

orbit_idl_LDADD = \
	$(top_builddir)/libIDL/libIDL.la \
	$(GLIB_LIBS) $(POPT_LIBS) -lm

orbit_idl_SOURCES = \
	orbit-idl-main.c
