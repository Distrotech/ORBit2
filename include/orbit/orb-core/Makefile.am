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
	corba-defs.h

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
IDLOUT = corba-defs.h

$(IDLOUT) runidl: $(top_srcdir)/src/orb/orb-core/corba-defs.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) -I$(top_srcdir)/src/idl/CORBA_PIDL -I$(top_srcdir)/src/idl/CORBA --noskels --nodefskels --nostubs \
                --showcpperrors $(top_srcdir)/src/orb/orb-core/corba-defs.idl
