lib_LTLIBRARIES = liborbitcpp.la

INCLUDES = \
	-I. \
	-I$(top_srcdir)/include \
	-DORBIT2_INTERNAL_API \
	$(WARN_CFLAGS) \
	$(ORBIT_CFLAGS)

liborbitcppincludedir = $(includedir)/orb

liborbitcppinclude_HEADERS = \
			orbitcpp.hh \
			orbitcpp_config.hh \
			orbitcpp_constants.hh \
			orbitcpp_exception.hh \
			orbitcpp_exceptionlist.hh \
			orbitcpp_object.hh \
			orbitcpp_orb.hh \
			orbitcpp_poa.hh \
			orbitcpp_sequence.hh \
			orbitcpp_smartptr.hh \
			orbitcpp_var_smartptr.hh \
			orbitcpp_tools.hh \
			orbitcpp_types.hh \
			orbitcpp_any.hh \
			orbitcpp_typecode.hh

liborbitcpp_la_SOURCES = \
			orbitcpp_types.cc \
			orbitcpp_exception.cc \
			orbitcpp_tools.cc \
			orbitcpp_object.cc \
			orbitcpp_orb.cc \
			orbitcpp_poa.cc \
			orbitcpp_constants.cc \
			orbitcpp_any.cc \
			orbitcpp_typecode.cc
			$(liborbitcppinclude_HEADERS)

liborbitcpp_la_LDFLAGS = -version-info $(LT_VERSION)
liborbitcpp_la_LIBADD = $(ORBIT_LIBS) -lstdc++
