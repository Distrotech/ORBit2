INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)
bin_PROGRAMS=ior-decode
noinst_PROGRAMS=test1 \
	echo-client echo-server \
	empty-client empty-server

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

$(ECHO_IDLOUT): echo.idl $(IDL)
	$(IDL) --showcpperrors $(srcdir)/echo.idl
	touch echo_built

$(EMPTY_IDLOUT): empty.idl $(IDL)
	$(IDL) --showcpperrors $(srcdir)/empty.idl
	touch empty_built

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
$(IDLOUT): test1.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(srcdir)/test1.idl

CLEANFILES=echo_built empty_built
