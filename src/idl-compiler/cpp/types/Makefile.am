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

headers = 		\
  IDLArray.hh		\
  IDLArrayList.hh	\
  IDLAttribAccessor.hh	\
  IDLBoolean.hh		\
  IDLCompound.hh	\
  IDLEnum.hh		\
  IDLElement.hh		\
  IDLException.hh	\
  IDLInterface.hh	\
  IDLMethod.hh		\
  IDLObject.hh		\
  IDLOperation.hh	\
  IDLScope.hh		\
  IDLSequence.hh	\
  IDLSimpleType.hh	\
  IDLString.hh		\
  IDLStruct.hh		\
  IDLType.hh		\
  IDLTypeCode.hh	\
  IDLTypedef.hh		\
  IDLUserDefScopeType.hh \
  IDLUserDefSimpleType.hh \
  IDLUserDefType.hh	\
  IDLVoid.hh

modules =		\
  IDLArray.cc		\
  IDLArrayList.cc	\
  IDLAttribAccessor.cc	\
  IDLBoolean.cc		\
  IDLCompound.cc	\
  IDLEnum.cc		\
  IDLElement.cc		\
  IDLException.cc	\
  IDLInterface.cc	\
  IDLMethod.cc		\
  IDLObject.cc		\
  IDLOperation.cc	\
  IDLScope.cc		\
  IDLSequence.cc	\
  IDLSimpleType.cc	\
  IDLString.cc		\
  IDLStruct.cc		\
  IDLType.cc		\
  IDLTypeCode.cc	\
  IDLTypedef.cc		\
  IDLUserDefScopeType.cc \
  IDLUserDefSimpleType.cc \
  IDLUserDefType.cc	\
  IDLVoid.cc

libidl_cpp_types_la_SOURCES = \
	$(headers) $(modules)

libidl_cpp_types_la_LIBADD = -lstdc++

endif
