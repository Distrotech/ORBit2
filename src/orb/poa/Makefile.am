noinst_LTLIBRARIES=liborb-poa.la

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
INCLUDES=-I$(top_srcdir)/include -I$(top_builddir)/include \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)
liborb_poa_la_SOURCES=poa.c $(POA_IDLOUT)
POA_IDLOUT=poa-defs-common.c
BUILT_SOURCES=$(POA_IDLOUT)

$(POA_IDLOUT) runidl_poa: poa-defs.idl $(IDL)
	-(rm -f $(POA_IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL \
	--define=Object=OObject --define=TypeCode=TTypeCode \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/idl/misc \
	--noskels --nodefskels --nostubs \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/poa-defs.idl
	for I in $(POA_IDLOUT); do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done
	mv -f poa-defs.h $(top_builddir)/include/orbit/orb-core/poa-defs.h
