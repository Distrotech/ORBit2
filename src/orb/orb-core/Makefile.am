noinst_LTLIBRARIES=liborb-core.la

INCLUDES=-I. -I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-I$(top_builddir)/include/orbit/orb-core \
	@GLIB_CFLAGS@ \
	@LINC_CFLAGS@

liborb_core_la_SOURCES=\
	$(IDLOUT) \
	$(POA_IDLOUT) \
	corba-orb.c \
	orbhttp.c \
	orbhttp.h \
	orbit-object.c

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
IDLOUT = corba-defs-common.c
POA_IDLOUT = poa-defs-common.c

$(IDLOUT) runidl: corba-defs.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL -I$(top_srcdir)/src/idl/CORBA --noskels --nodefskels --nostubs \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/corba-defs.idl
	mv -f corba-defs.h $(top_builddir)/include/orbit/orb-core/corba-defs.h

$(POA_IDLOUT) runidl_poa: poa-defs.idl $(IDL)
	-(rm -f $(POA_IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/idl/misc \
	--noskels --nodefskels --nostubs \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/poa-defs.idl
	mv -f poa-defs.h $(top_builddir)/include/orbit/orb-core/poa-defs.h
