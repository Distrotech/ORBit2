include $(top_srcdir)/test/cpp/Makefile.am_fragment

SUBDIRS=generated
noinst_PROGRAMS=client server

# Server is linked with stubs and skels:
server_SOURCES= server.cc struct_simple-impl.cc struct_simple.idl struct_simple-impl.hh
server_LDADD= generated/libstruct_simpleSkels.a generated/libstruct_simpleStubs.a $(testcpp_libs)

# Client is linked with skels:
client_SOURCES= client.cc struct_simple.idl
client_LDADD= generated/libstruct_simpleStubs.a $(testcpp_libs)

