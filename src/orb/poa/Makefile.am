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

POA_IDLOUT=poa-defs-common.c

$(main_src) : $(BUILT_SOURCE)

main_src = \
	poa.c \
	orbit-poa-export.h \
	orbit-poa.h \
	poa-manager.c \
	poa-policy.c \
	poa-servants.c

liborb_poa_la_SOURCES=\
	$(POA_IDLOUT) \
	$(main_src)

EXTRA_DIST=poa-defs.idl
BUILT_SOURCES=$(POA_IDLOUT)
CLEANFILES=$(BUILT_SOURCES)

$(POA_IDLOUT): poa-defs.idl $(IDL)
	-(rm -f $(POA_IDLOUT) || true) > /dev/null
	$(IDL) --oldstyle -I$(top_srcdir)/src/idl/CORBA_PIDL \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/idl/misc \
	-I$(top_srcdir)/src/orb/orb-core \
	--noskels --nodefskels --nostubs \
	--define=Object=OObject --define=TypeCode=TTypeCode \
                --showcpperrors $(top_srcdir)/src/orb/poa/poa-defs.idl
	for I in $(POA_IDLOUT) poa-defs.h; do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done
