INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)
noinst_PROGRAMS=test1

test1_SOURCES=test1.c \
	$(IDLOUT)

test1_LDADD=$(top_builddir)/src/orb/libORBit.la

test1.c: test1.h

IDLOUT=test1-stubs.c test1-skels.c test1-common.c test1.h

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
$(IDLOUT): test1.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(srcdir)/test1.idl
