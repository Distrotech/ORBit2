idlname = helloworld

include $(top_srcdir)/test/cpp/Makefile_generated.am_fragment

noinst_LIBRARIES = libhelloworldStubs.a libhelloworldSkels.a

libhelloworldStubs_a_SOURCES= $(stub_sources)
libhelloworldStubs_a_LIBADD= $(testcpp_generated_libs)

libhelloworldSkels_a_SOURCES= $(skel_sources)
libhelloworldSkels_a_LIBADD= $(testcpp_generated_libs)

