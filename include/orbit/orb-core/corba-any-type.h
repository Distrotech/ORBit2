#ifndef CORBA_ANY_TYPE_H
#define CORBA_ANY_TYPE_H 1

#include <orbit/orb-core/corba-pobj.h>

struct _CORBA_any {
	CORBA_TypeCode  _type;
	gpointer        _value;
	CORBA_boolean   _release;
};

#endif
