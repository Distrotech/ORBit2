INCLUDES = -I..			              \
	-I$(top_builddir)                     \
	-I$(top_builddir)/include             \
	 -I$(top_srcdir)                      \
	-I$(top_srcdir)/include               \
	-DVERSION=\"$(ORBIT_VERSION)\"        \
	-DORBIT2_INTERNAL_API                 \
	$(WARN_CFLAGS)                        \
	$(ORBIT_IDL_CFLAGS)

if ENABLE_CPP

noinst_LTLIBRARIES = libidl-cpp-types.la

headers = \
  IDLAny.hh \
  IDLArray.hh \
  IDLArrayList.hh \
  IDLBoolean.hh \
  IDLCompound.hh \
  IDLEnum.hh \
  IDLException.hh \
  IDLInterface.hh \
  IDLObject.hh \
  IDLSequence.hh \
  IDLSequenceList.hh \
  IDLSimpleType.hh \
  IDLString.hh \
  IDLStruct.hh \
  IDLType.hh \
  IDLTypeCode.hh \
  IDLUnion.hh \
  IDLUserDefScopeType.hh \
  IDLUserDefSimpleType.hh \
  IDLUserDefType.hh \
  IDLVoid.hh

modules = \
  IDLAny.cc \
  IDLArray.cc \
  IDLArrayList.cc \
  IDLBoolean.cc \
  IDLCompound.cc \
  IDLEnum.cc \
  IDLException.cc \
  IDLInterface.cc \
  IDLObject.cc \
  IDLSequence.cc \
  IDLSequenceList.cc \
  IDLSimpleType.cc \
  IDLString.cc \
  IDLStruct.cc \
  IDLType.cc \
  IDLTypeCode.cc \
  IDLUnion.cc \
  IDLUserDefScopeType.cc \
  IDLUserDefSimpleType.cc \
  IDLUserDefType.cc \
  IDLVoid.cc

libidl_cpp_types_la_SOURCES = \
	$(headers) $(modules)

libidl_cpp_types_la_LIBADD = -lstdc++

endif
