#ifndef CORBA_ANY_TYPE_H
#define CORBA_ANY_TYPE_H 1

#include <orbit/orb-core/corba-pobj.h>

struct CORBA_any_struct {
	CORBA_TypeCode  _type;
	gpointer        _value;
	CORBA_boolean   _release;
};

#endif
