noinst_LTLIBRARIES=liborb-poa.la

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-I$(top_builddir)/src/orb/orb-core \
	-I$(top_builddir)/include/orbit/poa \
	-I$(top_srcdir)/include/orbit/orb-core \
	-I$(top_builddir)/include/orbit/orb-core \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)

POA_IDLOUT=poa-defs-common.c poa-defs.h

liborb_poa_la_SOURCES=\
	poa.c \
	$(POA_IDLOUT) \
	orbit-poa-export.h \
	orbit-poa.h \
	poa-manager.c \
	poa-policy.c \
	poa-servants.c

EXTRA_DIST=poa-defs.idl
BUILT_SOURCES=$(POA_IDLOUT)
CLEANFILES=runidl_poa $(BUILT_SOURCES)

$(POA_IDLOUT): runidl_poa

runidl_poa: poa-defs.idl $(IDL)
	-(rm -f $(POA_IDLOUT) || true) > /dev/null
	$(IDL) --oldstyle -I$(top_srcdir)/src/idl/CORBA_PIDL \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/idl/misc \
	-I$(top_srcdir)/src/orb/orb-core \
	--noskels --nodefskels --nostubs \
	--define=Object=OObject --define=TypeCode=TTypeCode \
                --showcpperrors $(top_srcdir)/src/orb/poa/poa-defs.idl
	for I in $(POA_IDLOUT); do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done
	mv -f poa-defs.h $(top_builddir)/include/orbit/poa/poa-defs.h
	touch runidl_poa

