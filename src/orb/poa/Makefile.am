noinst_LTLIBRARIES=liborb-poa.la

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	-I$(top_builddir)/src/orb/orb-core \
	-I$(top_srcdir)/src/orb/orb-core \
	-I$(top_builddir)/include/orbit/poa \
	-I$(top_srcdir)/include/orbit/orb-core \
	-I$(top_builddir)/include/orbit/orb-core \
	-I$(top_srcdir)/src/orb/poa \
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
	poa-servants.c \
	orbit-adaptor.c

liborb_poa_la_SOURCES=\
	$(POA_IDLOUT) \
	$(main_src)

EXTRA_DIST=poa-defs.idl
BUILT_SOURCES=$(POA_IDLOUT)
CLEANFILES=$(BUILT_SOURCES)

IDL_FLAGS= -I$(top_srcdir)/src/idl/CORBA_PIDL			\
	-I$(top_srcdir)/src/idl/CORBA				\
	-I$(top_srcdir)/src/idl/misc				\
	-I$(top_srcdir)/src/orb/orb-core			\
	--noskels --nodefskels --nostubs --noidata --noheaders  \
	--define=Object=OObject --define=TypeCode=TTypeCode	\
        --showcpperrors
IDL_FILES=poa-defs.idl
include $(top_srcdir)/Makefile.shared
