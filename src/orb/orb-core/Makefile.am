noinst_LTLIBRARIES=liborb-core.la

INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	@GLIB_CFLAGS@ \
	@LINC_CFLAGS@

liborb_core_la_SOURCES=\
	$(IDLOUT) \
	corba-orb.c \
	orbhttp.c \
	orbhttp.h \
	orbit-object.c

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
IDLOUT = corba-defs-common.c corba-defs-stubs.c

$(IDLOUT) runidl: corba-defs.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL -I$(top_srcdir)/src/idl/CORBA --noskels --nodefskels \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/corba-defs.idl
	[ -f $(top_builddir)/include/orbit/orb-core/corba-defs.h ] \
	&& rm -f corba-defs.h \
	|| mv -f corba-defs.h $(top_builddir)/include/orbit/orb-core/corba-defs.h
