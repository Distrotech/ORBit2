noinst_LTLIBRARIES=liborb-core.la

TYPELIB_DIR=$(libdir)/orbit

INCLUDES=-I. -I$(srcdir) \
	-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-I$(top_builddir)/include/orbit/orb-core \
	-DORBIT_TYPELIB_DIR=\""$(TYPELIB_DIR)"\" \
	@GLIB_CFLAGS@ \
	@LINC_CFLAGS@

main_src = \
	corba-orb.c \
	orbhttp.c \
	orbhttp.h \
	orbit-object.c \
	orbit-small.c \
	orbit-trace.c \
	corba-object.c \
	corba-policy.c \
	corba-env.c \
	corba-string.c \
	allocators.c \
	corba-typecode.c \
	corba-types.c \
	corba-any.c \
	corba-context.c \
	orb-core-private.h \
	orb-core-export.h \
	corba-nvlist.c \
	corba-request.c \
	iop-profiles.c \
	iop-profiles.h

liborb_core_la_SOURCES= \
	$(IOP_IDLOUT) \
	$(IDLOUT) \
	$(OPS_IDLOUT) \
	$(IFACE_IDLOUT) \
	$(main_src)

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
IDLOUT=corba-defs-common.c
IOP_IDLOUT=iop-defs-common.c
OPS_IDLOUT=corba-ops-stubs.c corba-ops-common.c corba-ops.h corba-ops-skels.c
IFACE_IDLOUT=orbit-interface.h orbit-interface-common.c

$(main_src) : $(BUILT_SOURCES)

$(top_builddir)/include/orbit/orb-core/corba-defs.h $(IDLOUT): corba-defs.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL -I$(top_srcdir)/src/idl/CORBA \
	--noskels --nodefskels --nostubs --noidata \
	--define=Object=OObject --define=TypeCode=TTypeCode \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/corba-defs.idl
	for I in $(IDLOUT) corba-defs.h; do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done; \
	cp -f corba-defs.h $(top_builddir)/include/orbit/orb-core/corba-defs.h;

$(IOP_IDLOUT): iop-defs.idl $(IDL)
	-(rm -f $(IOP_IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL \
	--define=Object=OObject --define=TypeCode=TTypeCode \
	-I$(top_srcdir)/src/idl/CORBA \
	-I$(top_srcdir)/src/idl/misc \
	-I$(top_srcdir)/src/idl/interop \
	--noskels --nodefskels --nostubs --noidata \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/iop-defs.idl
	for I in $(IOP_IDLOUT) iop-defs.h; do \
	sed -e 's,OObject,Object,g' -e 's,TTypeCode,TypeCode,g' $$I > $$I.out; \
	mv $$I.out $$I; done; \
	mv -f iop-defs.h $(top_builddir)/include/orbit/orb-core/iop-defs.h;

$(OPS_IDLOUT): corba-ops.idl $(IDL)
	-(rm -f $(OPS_IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(top_srcdir)/src/orb/orb-core/corba-ops.idl
	for I in $(OPS_IDLOUT); do \
		sed -e 's,ZZZis_a,_is_a,g' -e 's,ZZis_a,is_a,g' $$I > $$I.out; mv $$I.out $$I; \
	done; \
	sed -e "s,Z,_,g" corba-ops-skels.c > corba-ops-skels.c.out; \
	mv corba-ops-skels.c.out corba-ops-skels.c;

# This has to be built and installed before we can compile
# the other bits, since the generated code includes orbit-interface.h
$(IDLOUT) $(IOP_IDLOUT) $(OPS_IDLOUT) : $(IFACE_IDLOUT)

$(top_builddir)/include/orbit/orb-core/orbit-interface.h $(IFACE_IDLOUT) : orbit-interface.idl $(IDL)
	-(rm -f $(IFACE_IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(top_srcdir)/src/orb/orb-core/orbit-interface.idl
	cp -f orbit-interface.h $(top_builddir)/include/orbit/orb-core/orbit-interface.h; \
	rm -f orbit-interface-stubs.c orbit-interface-skels.c

BUILT_SOURCES=$(IDLOUT) $(IOP_IDLOUT) $(OPS_IDLOUT) $(IFACE_IDLOUT)
CLEANFILES=$(BUILT_SOURCES)
EXTRA_DIST=corba-defs.idl iop-defs.idl corba-ops.idl orbit-interface.idl
