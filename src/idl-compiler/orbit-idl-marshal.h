#ifndef ORBIT_IDL_MARSHAL_H
#define ORBIT_IDL_MARSHAL_H 1

#include "orbit-idl2.h"

typedef enum { MARSHAL_INLINE=1<<0, MARSHAL_FUNC=1<<1, MARSHAL_ANY=1<<2,
	       MARSHAL_ALL=0xFFFF } OIDL_Marshal_Method;
#define MARSHAL_NUM 3

typedef struct {
  OIDL_Marshal_Method mtype, dmtype;
  int use_count;
  int size;
} OIDL_Type_Marshal_Info;

typedef struct {
  GHashTable *type_marshal_info;
} OIDL_Marshal_Context;

OIDL_Marshal_Context *oidl_marshal_context_new(IDL_tree tree);
void oidl_marshal_context_dump(OIDL_Marshal_Context *ctxt);
void oidl_marshal_context_free(OIDL_Marshal_Context *ctxt);

#endif
