noinst_LTLIBRARIES=liborb-dynamic.la

DYNAMIC_IDLOUT=dynamic-defs-common.c
liborb_dynamic_la_SOURCES= \
	$(DYNAMIC_IDLOUT) \
	dynany.c

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
INCLUDES= \
	-I$(top_srcdir)/include \
	-I$(top_srcdir)/src/orb \
	-I$(top_builddir)/include \
	-I$(top_builddir)/include/orbit/dynamic \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)

DYNAMIC_IDLOUT=dynamic-defs-common.c dynamic-defs.h

BUILT_SOURCES=$(DYNAMIC_IDLOUT)

$(DYNAMIC_IDLOUT) runidl_dynamic: dynamic-defs.idl $(IDL)
	-(rm -f $(DYNAMIC_IDLOUT) || true) > /dev/null
	$(IDL) --noidata -I$(top_srcdir)/src/idl/CORBA_PIDL \
	--define=Object=OObject --define=TypeCode=TTypeCode \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/idl/misc \
	-I$(top_srcdir)/src/orb/orb-core \
	--noskels --nodefskels --nostubs --noidata \
                --showcpperrors $(top_srcdir)/src/orb/dynamic/dynamic-defs.idl
	for I in $(DYNAMIC_IDLOUT); do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done

EXTRA_DIST=dynamic-defs.idl
