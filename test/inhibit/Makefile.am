noinst_PROGRAMS=\
	test-inhibit

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)

LDADD = $(top_builddir)/src/orb/libORBit-2.la

FOO_IDLOUT=foo.h foo-stubs.c foo-skels.c foo-common.c
BAA_IDLOUT=baa.h baa-stubs.c baa-skels.c baa-common.c

test_inhibit_SOURCES=$(FOO_IDLOUT) $(BAA_IDLOUT) test-inhibit.c

$(FOO_IDLOUT): foo_built

foo_built: foo.idl $(IDL)
	-(rm -f $(FOO_IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(srcdir)/foo.idl
	touch foo_built

$(BAA_IDLOUT): baa_built
baa_built: baa.idl $(IDL)
	-(rm -f $(BAA_IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(srcdir)/baa.idl
	touch baa_built

TESTS=test-inhibit

BUILT_SOURCES=$(BAA_IDLOUT) $(FOO_IDLOUT)
CLEANFILES=foo_built baa_built test1_built $(BUILT_SOURCES)

EXTRA_DIST=foo.idl baa.idl
