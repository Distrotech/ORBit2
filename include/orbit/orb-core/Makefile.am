orbcoreincludedir=$(includedir)/orbit/orb-core
orbcoreinclude_HEADERS= \
	corba-environment-type.h	\
	corba-ir.h			\
	corba-nvlist-type.h		\
	corba-nvlist.h			\
	corba-object-type.h		\
	corba-orb-type.h		\
	corba-orb.h			\
	corba-pobj.h			\
	corba-typecode-type.h		\
	orb-core.h			\
	orb-defs.h			\
	orb-types.h			\
	orbit-object.h			\
	orbit-small.h			\
	corba-defs.h			\
	corba-context.h			\
	corba-context-type.h		\
	iop-defs.h			\
	corba-object.h			\
	corba-policy-type.h		\
	corba-string.h			\
	corba-environment.h		\
	corba-any.h			\
	orb-core-types.h		\
	corba-typecode.h		\
	allocators.h			\
	corba-any-type.h		\
	orbit-interface.h

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
IDLOUT = corba-defs.h iop-defs.h

$(IDLOUT) runidl: $(top_srcdir)/src/orb/orb-core/corba-defs.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL -I$(top_srcdir)/src/idl/CORBA --noskels --nodefskels --nostubs --nocommon \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/corba-defs.idl
