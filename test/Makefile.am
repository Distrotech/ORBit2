INCLUDES=-I$(top_srcdir)/include \
	-I$(top_builddir)/include \
	$(LINC_CFLAGS) \
	$(GLIB_CFLAGS)
noinst_PROGRAMS=test1 echo-client echo-server

ECHO_IDLOUT=echo.h echo-common.c echo-stubs.c echo-skels.c
echo_client_SOURCES=echo-client.c $(ECHO_IDLOUT)
echo_client_LDADD=$(top_builddir)/src/orb/libORBit.la
echo_server_SOURCES=echo-server.c echo-srv.c $(ECHO_IDLOUT)
echo_server_LDADD=$(top_builddir)/src/orb/libORBit.la
$(srcdir)/echo-local.c $(srcdir)/echo-server.c $(srcdir)/echo-client.c: echo.h
	@:

IDLOUT=test1-stubs.c test1-skels.c test1-common.c test1.h
test1_SOURCES=test1.c $(IDLOUT)
test1_LDADD=$(top_builddir)/src/orb/libORBit.la
$(srcdir)/test1.c: test1.h

$(ECHO_IDLOUT): echo.idl $(IDL)
	$(IDL) --showcpperrors $(srcdir)/echo.idl
	touch echo_built

IDL = $(top_builddir)/src/idl-compiler/orbit-idl
$(IDLOUT): test1.idl $(IDL)
	-(rm -f $(IDLOUT) || true) > /dev/null
	$(IDL) --showcpperrors $(srcdir)/test1.idl
