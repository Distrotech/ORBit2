noinst_HEADERS=runidl_poa runidl_dynamic
IDL = $(top_builddir)/src/idl-compiler/orbit-idl

runidl_poa: $(top_srcdir)/src/orb/poa/poa-defs.idl $(IDL)
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/orb/orb-core \
	-I$(top_srcdir)/src/idl/misc \
	--define=Object=OObject --define=TypeCode=TTypeCode \
	--noskels --nodefskels --nostubs --nocommon \
                --showcpperrors $(top_srcdir)/src/orb/poa/poa-defs.idl
	for I in poa-defs.h; do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done
	mv -f poa-defs.h $(top_builddir)/include/orbit/poa/poa-defs.h
	touch runidl_poa

runidl_dynamic: $(top_srcdir)/src/orb/dynamic/dynamic-defs.idl $(IDL)
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/orb/orb-core \
	-I$(top_srcdir)/src/idl/misc \
	--define=Object=OObject --define=TypeCode=TTypeCode \
	--noskels --nodefskels --nostubs --nocommon \
                --showcpperrors $(top_srcdir)/src/orb/dynamic/dynamic-defs.idl
	for I in dynamic-defs.h; do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done
	mv -f dynamic-defs.h $(top_builddir)/include/orbit/dynamic/dynamic-defs.h
	touch runidl_dynamic

BUILT_SOURCES=runidl_poa runidl_dynamic
CLEANFILES=runidl_poa runidl_dynamic
