IDL = $(top_builddir)/src/idl-compiler/orbit-idl
INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)
bin_PROGRAMS=ior-decode
noinst_PROGRAMS=test1 \
	echo-client echo-server \
	empty-client empty-server \
	test-any-client test-any-server

TEST_ANY_IDLOUT=test-any.h test-any-common.c test-any-stubs.c test-any-skels.c
test_any_client_SOURCES=test-any-client.c $(TEST_ANY_IDLOUT)
test_any_client_LDADD=$(top_builddir)/src/orb/libORBit.la
test_any_server_SOURCES=test-any-server.c $(TEST_ANY_IDLOUT)
test_any_server_LDADD=$(top_builddir)/src/orb/libORBit.la
$(srcdir)/test-any-client.c $(srcdir)/test-any-server.c: test-any.h

ECHO_IDLOUT=echo.h echo-common.c echo-stubs.c echo-skels.c
echo_client_SOURCES=echo-client.c $(ECHO_IDLOUT)
echo_client_LDADD=$(top_builddir)/src/orb/libORBit.la
echo_server_SOURCES=echo-server.c echo-srv.c $(ECHO_IDLOUT)
echo_server_LDADD=$(top_builddir)/src/orb/libORBit.la
$(srcdir)/echo-client.c $(srcdir)/echo-server.c $(srcdir)/echo-srv.c: echo.h

EMPTY_IDLOUT=empty.h empty-common.c empty-stubs.c empty-skels.c
empty_client_SOURCES=empty-client.c $(EMPTY_IDLOUT)
empty_client_LDADD=$(top_builddir)/src/orb/libORBit.la
empty_server_SOURCES=empty-server.c $(EMPTY_IDLOUT)
empty_server_LDADD=$(top_builddir)/src/orb/libORBit.la
$(srcdir)/empty-client.c $(srcdir)/empty-server.c: empty.h

IDLOUT=test1-stubs.c test1-skels.c test1-common.c test1.h
test1_SOURCES=test1.c $(IDLOUT)
test1_LDADD=$(top_builddir)/src/orb/libORBit.la
$(srcdir)/test1.c: test1.h

ior_decode_SOURCES=ior-decode.c
ior_decode_LDADD=$(top_builddir)/src/orb/libORBit.la

$(ECHO_IDLOUT): echo_built

echo_built: echo.idl $(IDL)
	$(IDL) --showcpperrors $(srcdir)/echo.idl
	touch echo_built

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

BUILT_SOURCES=$(EMPTY_IDLOUT) $(ECHO_IDLOUT) $(IDLOUT) $(TEST_ANY_IDLOUT)
CLEANFILES=echo_built empty_built test1_built test_any_built $(BUILT_SOURCES)
