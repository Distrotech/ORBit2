#ifndef ORB_CORE_PRIVATE_H
#define ORB_CORE_PRIVATE_H 1

#include <orbit/orbit.h>

CORBA_TypeCode ORBit_get_union_tag(CORBA_TypeCode union_tc, gconstpointer *val,
				   gboolean update);
size_t ORBit_gather_alloc_info(CORBA_TypeCode tc);
gint ORBit_find_alignment(CORBA_TypeCode tc);
void ORBit_copy_value_core(gconstpointer *val, gpointer *newval, CORBA_TypeCode tc);

#endif
