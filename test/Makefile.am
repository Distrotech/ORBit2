SUBDIRS = everything inhibit

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)
bin_PROGRAMS=ior-decode
noinst_PROGRAMS=test1 \
	echo-client echo-server \
	empty-client empty-server \
	test-any-client test-any-server \
	test-dynany

TESTS = test-dynany

LDADD = $(top_builddir)/src/orb/libORBit-2.la

TEST_ANY_IDLOUT=test-any.h test-any-common.c test-any-stubs.c test-any-skels.c
test_any_client_SOURCES=test-any-client.c $(TEST_ANY_IDLOUT)
test_any_server_SOURCES=test-any-server.c $(TEST_ANY_IDLOUT)
$(srcdir)/test-any-client.c $(srcdir)/test-any-server.c: test-any.h

ECHO_IDLOUT=echo.h echo-common.c echo-stubs.c echo-skels.c
echo_client_SOURCES=echo-client.c $(ECHO_IDLOUT)
echo_server_SOURCES=echo-server.c echo-srv.c $(ECHO_IDLOUT) echo-share.h
$(srcdir)/echo-client.c $(srcdir)/echo-server.c $(srcdir)/echo-srv.c: echo.h

EMPTY_IDLOUT=empty.h empty-common.c empty-stubs.c empty-skels.c
empty_client_SOURCES=empty-client.c $(EMPTY_IDLOUT)
empty_server_SOURCES=empty-server.c $(EMPTY_IDLOUT)
$(srcdir)/empty-client.c $(srcdir)/empty-server.c: empty.h

IDLOUT=test1-stubs.c test1-skels.c test1-common.c test1.h
test1_SOURCES=test1.c $(IDLOUT)
$(srcdir)/test1.c: test1.h

ior_decode_SOURCES=ior-decode.c

DYNANY_IDLOUT=test-dynany.h dynany-common.c
test_dynany_SOURCES=$(DYNANY_IDLOUT) test-dynany.c
test_dynany_LDFLAGS=-lefence

$(ECHO_IDLOUT): echo_built

echo_built: echo.idl $(IDL)
	$(IDL) --showcpperrors $(srcdir)/echo.idl
	touch echo_built

$(DYNANY_IDLOUT): dynany_built

dynany_built: dynany.idl $(IDL)
	$(IDL) --noskels --nostubs --showcpperrors $(srcdir)/dynany.idl
	touch dynany_built

$(EMPTY_IDLOUT): empty_built
empty_built: empty.idl $(IDL)
	$(IDL) --showcpperrors $(srcdir)/empty.idl
	touch empty_built

$(IDLOUT): test1_built
test1_built: test1.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(srcdir)/test1.idl
	touch test1_built

$(TEST_ANY_IDLOUT): test_any_built
test_any_built: test-any.idl $(IDL)
	-(rm -f $(TEST_ANY_IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(srcdir)/test-any.idl
	touch test_any_built

BUILT_SOURCES=$(EMPTY_IDLOUT) $(ECHO_IDLOUT) $(IDLOUT) $(TEST_ANY_IDLOUT) $(DYNANY_IDLOUT)
CLEANFILES=dynany_built echo_built empty_built test1_built test_any_built $(BUILT_SOURCES)

EXTRA_DIST=echo.idl empty.idl test-any.idl test1.idl dynany.idl
