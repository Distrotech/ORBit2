lib_LTLIBRARIES = libORBit-2-cpp.la

INCLUDES = \
	-I. \
	-I$(top_srcdir)/include \
	-DORBIT2_INTERNAL_API \
	$(WARN_CFLAGS) \
	$(ORBIT_CFLAGS)

libORBit_2_cppincludedir = $(includedir)/orbit-2.0/orbit/orb-cpp

libORBit_2_cppinclude_HEADERS = 			\
			orbitcpp.hh 			\
			orbitcpp_config.hh 		\
			orbitcpp_constants.hh 		\
			orbitcpp_exception.hh 		\
			orbitcpp_exceptionlist.hh 	\
			orbitcpp_object.hh 		\
			orbitcpp_orb.hh 		\
			orbitcpp_poa.hh 		\
			orbitcpp_sequence.hh 		\
			orbitcpp_compound_seq.hh 	\
			orbitcpp_simple_seq.hh 		\
			orbitcpp_string_seq.hh		\
			orbitcpp_smartptr.hh 		\
			orbitcpp_var_smartptr.hh 	\
			orbitcpp_tools.hh 		\
			orbitcpp_types.hh 		\
			orbitcpp_any.hh 		\
			orbitcpp_typecode.hh

libORBit_2_cpp_la_SOURCES = \
			$(libORBit_2_cppinclude_HEADERS)	\
			orbitcpp_types.cc 			\
			orbitcpp_exception.cc 			\
			orbitcpp_tools.cc 			\
			orbitcpp_object.cc 			\
			orbitcpp_orb.cc 			\
			orbitcpp_poa.cc 			\
			orbitcpp_constants.cc 			\
			orbitcpp_any.cc 			\
			orbitcpp_typecode.cc
			$(libORBit_2_cppinclude_HEADERS)

libORBit_2_cpp_la_LDFLAGS = -version-info $(LT_VERSION)
libORBit_2_cpp_la_LIBADD = $(ORBIT_LIBS) -lstdc++
