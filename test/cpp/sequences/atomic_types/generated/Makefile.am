idlname = test

include $(top_srcdir)/test/cpp/Makefile_generated.am_fragment

noinst_LIBRARIES = libtestStubs.a libtestSkels.a

libtestStubs_a_SOURCES= $(stub_sources)
libtestStubs_a_LIBADD= $(testcpp_generated_libs)

libtestSkels_a_SOURCES= $(skel_sources)
libtestSkels_a_LIBADD= $(testcpp_generated_libs)

