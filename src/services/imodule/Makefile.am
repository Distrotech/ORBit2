NULL =

noinst_LTLIBRARIES = libORBit-imodule-2.la

INCLUDES =				\
        -I$(top_srcdir)/include		\
        -I$(top_builddir)/include	\
	-DORBIT2_INTERNAL_API		\
	$(WARN_CFLAGS)			\
	$(ORBIT_CFLAGS)			\
	$(ORBIT_IDL_CFLAGS)		\
	$(NULL)

LDADD = $(ORBIT_LIBS)				\
	$(ORBIT_IDL_LIBS)			\
	$(top_builddir)/src/orb/libORBit-2.la

libORBit_imodule_2_la_SOURCES =			\
	orbit-imodule-utils.c		\
	orbit-imodule-utils.h		\
	orbit-imodule-libidl-utils.c	\
	orbit-imodule-libidl-utils.h	\
	orbit-imodule.c			\
	orbit-imodule.h			\
	$(NULL)
