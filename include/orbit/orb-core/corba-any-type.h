#ifndef CORBA_ANY_TYPE_H
#define CORBA_ANY_TYPE_H 1

#include <orbit/orb-core/corba-pobj.h>

struct _CORBA_any {
  CORBA_TypeCode _type;
  gpointer _value;
  CORBA_boolean _release;
};

typedef struct ORBit_marshal_value_info_struct {
  CORBA_TypeCode alias_element_type;
} ORBit_marshal_value_info;

#endif
