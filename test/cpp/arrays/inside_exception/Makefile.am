include $(top_srcdir)/test/cpp/Makefile.am_fragment

SUBDIRS=generated
noinst_PROGRAMS=client server

# Server is linked with stubs and skels:
server_SOURCES= server.cc test-impl.cc test-impl.hh test.idl
server_LDADD= generated/libsimpleSkels.a generated/libsimpleStubs.a $(testcpp_libs)

# Client is linked with stubs:
client_SOURCES= client.cc test.idl
client_LDADD= generated/libsimpleStubs.a $(testcpp_libs)

