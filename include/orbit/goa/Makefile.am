goaincludedir = $(includedir)/orbit-2.0/orbit/goa

goainclude_HEADERS =		\
	goa-basics.h		\
	orbit-goa.h		\
	orbit-gservant.h	\
	gobject-adaptor.h

BUILT_SOURCES = gobject-adaptor.h

IDL_FLAGS =  -I$(top_srcdir)/src/idl/CORBA_PIDL				\
	-I$(top_srcdir)/src/idl/CORBA					\
	-I$(top_srcdir)/src/orb/orb-core				\
	-I$(top_srcdir)/src/idl/misc					\
	--define=Object=OObject --define=TypeCode=TTypeCode		\
	--noskels --nodefskels --nostubs --nocommon --noidata		\
        --showcpperrors
IDL_FILES =gobject-adaptor.idl
IDL_DIR = $(top_srcdir)/src/orb/goa/
include $(top_srcdir)/Makefile.shared

CLEANFILES=$(BUILT_SOURCES)
