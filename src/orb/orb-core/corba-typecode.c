#include "config.h"
#include <orbit/orbit.h>

const ORBit_RootObject_Interface ORBit_TypeCode_epv = {
  ORBIT_ROT_TYPECODE,
  NULL
};

#define DEF_TC_BASIC(nom) \
struct CORBA_TypeCode_struct TC_CORBA_##nom##_struct = { \
  {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC}, \
  CORBA_tk_##nom, \
  #nom, \
  "IDL:CORBA/" #nom ":1.0", \
  0, 0, NULL, NULL, NULL, CORBA_OBJECT_NIL, -1, 0, 0, 0 \
}

#define CORBA_tk_Object CORBA_tk_objref
#define CORBA_tk_unsigned_long CORBA_tk_ulong
#define CORBA_tk_long_long CORBA_tk_longlong
#define CORBA_tk_unsigned_long_long CORBA_tk_ulonglong
#define CORBA_tk_unsigned_short CORBA_tk_ushort

struct CORBA_TypeCode_struct TC_null_struct = {
  {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
  CORBA_tk_null,
  "null",
  "Null",
  0, 0, NULL, NULL, NULL, CORBA_OBJECT_NIL, -1, 0, 0, 0
};
struct CORBA_TypeCode_struct TC_void_struct = {
  {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
  CORBA_tk_void,
  "void",
  "IDL:CORBA/void:1.0",
  0, 0, NULL, NULL, NULL, CORBA_OBJECT_NIL, -1, 0, 0, 0
};

DEF_TC_BASIC(char);
DEF_TC_BASIC(wchar);
DEF_TC_BASIC(string);
DEF_TC_BASIC(long);
DEF_TC_BASIC(unsigned_long);
DEF_TC_BASIC(float);
DEF_TC_BASIC(double);
DEF_TC_BASIC(short);
DEF_TC_BASIC(unsigned_short);
DEF_TC_BASIC(boolean);
DEF_TC_BASIC(octet);
DEF_TC_BASIC(any);
DEF_TC_BASIC(TypeCode);
DEF_TC_BASIC(Principal);
DEF_TC_BASIC(Object);
DEF_TC_BASIC(wstring);
DEF_TC_BASIC(long_long);
DEF_TC_BASIC(unsigned_long_long);

gboolean
ORBit_TypeCode_demarshal_value(CORBA_TypeCode tc,
			       gpointer *val,
			       GIOPRecvBuffer *buf,
			       gboolean dup_strings,
			       CORBA_ORB orb)
{
  return TRUE;
}

gpointer
CORBA_TypeCode__freekids(gpointer mem, gpointer dat)
{
  CORBA_TypeCode t, *tp;

  tp = mem;
  t = *tp;

  CORBA_Object_release((CORBA_Object)t, NULL);

  return tp + 1;
}

void
ORBit_encode_CORBA_TypeCode(CORBA_TypeCode tc, GIOPSendBuffer* buf)
{
}

gboolean
ORBit_decode_CORBA_TypeCode(CORBA_TypeCode* tc, GIOPRecvBuffer* buf)
{
  return FALSE;
}
