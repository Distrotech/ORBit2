include $(top_srcdir)/test/cpp/Makefile.am_fragment

SUBDIRS=generated
noinst_PROGRAMS=client server

server_SOURCES= server.cc helloworld-impl.cc helloworld.idl helloworld-impl.hh
server_LDADD= generated/libhelloworldSkels.a $(LDADDS)

client_SOURCES= client.cc helloworld.idl
client_LDADD= generated/libhelloworldStubs.a $(LDADDS)

