NULL = 

noinst_LTLIBRARIES = liborb-goa.la

IDL = $(top_builddir)/src/idl-compiler/orbit-idl-2

INCLUDES =						\
	-I$(top_srcdir)/include				\
	-I$(top_builddir)/include			\
	-I$(top_builddir)/src/orb/orb-core		\
	-I$(top_srcdir)/src/orb/orb-core		\
	-I$(top_builddir)/include/orbit/poa		\
	-I$(top_builddir)/include/orbit/goa		\
	-I$(top_srcdir)/include/orbit/orb-core		\
	-I$(top_builddir)/include/orbit/orb-core	\
	-I$(top_srcdir)/src/orb/poa			\
	-DORBIT2_INTERNAL_API				\
	$(WARN_CFLAGS)					\
	$(ORBIT_DEBUG_CFLAGS)				\
	$(DISABLE_DEPRECATED_CFLAGS)			\
	$(ORBIT_CFLAGS)					\
	$(NULL)

GOA_IDLOUT = gobject-adaptor-common.c

liborb_goa_la_SOURCES =			\
	$(GOA_IDLOUT)			\
	orbit-goa.c			\
	orbit-gservant.c		\
	$(NULL)

IDL_FLAGS =							\
	-I$(top_srcdir)/src/idl/CORBA_PIDL			\
	-I$(top_srcdir)/src/idl/CORBA				\
	-I$(top_srcdir)/src/idl/misc				\
	-I$(top_srcdir)/src/orb/orb-core			\
	--noskels --nodefskels --nostubs --noidata --noheaders	\
	--define=Object=OObject --define=TypeCode=TTypeCode	\
        --showcpperrors						\
	$(NULL)

IDL_FILES = gobject-adaptor.idl
include $(top_srcdir)/Makefile.shared

BUILT_SOURCES = $(GOA_IDLOUT)
CLEANFILES    = $(BUILT_SOURCES)
EXTRA_DIST    = $(IDL_FILES)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)
