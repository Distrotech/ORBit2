noinst_LTLIBRARIES=liborb-core.la

INCLUDES=-I. -I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-I$(top_builddir)/include/orbit/orb-core \
	@GLIB_CFLAGS@ \
	@LINC_CFLAGS@

liborb_core_la_SOURCES=\
	$(IDLOUT) \
	$(POA_IDLOUT) \
	$(IOP_IDLOUT) \
	corba-orb.c \
	orbhttp.c \
	orbhttp.h \
	orbit-object.c

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
IDLOUT = corba-defs-common.c
POA_IDLOUT = poa-defs-common.c
IOP_IDLOUT = iop-defs-common.c

$(IDLOUT) runidl: corba-defs.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL -I$(top_srcdir)/src/idl/CORBA \
	--noskels --nodefskels --nostubs \
	--define=Object=OObject --define=TypeCode=TTypeCode \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/corba-defs.idl
	for I in $(IDLOUT) corba-defs.h; do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done
	mv -f corba-defs.h $(top_builddir)/include/orbit/orb-core/corba-defs.h

$(POA_IDLOUT) runidl_poa: poa-defs.idl $(IDL)
	-(rm -f $(POA_IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL \
	--define=Object=OObject --define=TypeCode=TTypeCode \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/idl/misc \
	--noskels --nodefskels --nostubs \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/poa-defs.idl
	for I in $(POA_IDLOUT) poa-defs.h; do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done
	mv -f poa-defs.h $(top_builddir)/include/orbit/orb-core/poa-defs.h

$(IOP_IDLOUT) runidl_iop: iop-defs.idl $(IDL)
	-(rm -f $(IOP_IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL \
	--define=Object=OObject --define=TypeCode=TTypeCode \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/idl/misc \
	-I$(top_srcdir)/src/idl/interop \
	--noskels --nodefskels --nostubs \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/iop-defs.idl
	for I in $(IOP_IDLOUT) iop-defs.h; do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done
	mv -f iop-defs.h $(top_builddir)/include/orbit/orb-core/iop-defs.h
