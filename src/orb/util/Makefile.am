noinst_LTLIBRARIES=liborb-util.la

INCLUDES =                                                  \
	-I$(top_srcdir)/include                             \
	-I$(top_builddir)/include                           \
	-DORBIT_SYSTEM_RCFILE=\"$(sysconfdir)/orbitrc\"     \
	$(ORBIT_CFLAGS)

liborb_util_la_SOURCES= \
	genrand.c       \
	genrand.h       \
	orbit-options.c \
	orbit-options.h \
	orbit-util.c
