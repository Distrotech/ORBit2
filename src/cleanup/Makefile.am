bin_PROGRAMS=orbit-cleanup-sockets

INCLUDES =						\
	-I.						\
	-I$(srcdir)					\
	-I$(top_srcdir)/include				\
	-I$(top_builddir)/include			\
	$(WARN_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)			\
	$(ORBIT_CFLAGS)

orbit_cleanup_sockets_SOURCES = cleanup.c
orbit_cleanup_LDADD = $(ORBIT_LIBS)
