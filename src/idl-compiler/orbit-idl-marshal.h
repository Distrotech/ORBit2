#ifndef ORBIT_IDL_MARSHAL_H
#define ORBIT_IDL_MARSHAL_H 1

#include "orbit-idl3-types.h"

typedef enum { MARSHAL_INLINE=1<<0, MARSHAL_FUNC=1<<1, MARSHAL_ANY=1<<2,
	       MARSHAL_ALL=0xFFFF } OIDL_Marshal_Method;
#define MARSHAL_NUM 3
#define INLINE_SIZE_THRESHOLD 5

typedef struct {
  OIDL_Marshal_Method mtype, dmtype, avail_mtype, avail_dmtype;
  int use_count;
  int size;
  IDL_tree tree;
} OIDL_Type_Marshal_Info;

typedef struct {
  IDL_tree tree;
} OIDL_Marshal_Tree_Entry;

struct _OIDL_Marshal_Context {
  GHashTable *type_marshal_info;
};

OIDL_Marshal_Context *oidl_marshal_context_new(IDL_tree tree);
void oidl_marshal_context_dump(OIDL_Marshal_Context *ctxt);
void oidl_marshal_context_free(OIDL_Marshal_Context *ctxt);
OIDL_Type_Marshal_Info *oidl_marshal_context_find(OIDL_Marshal_Context *ctxt, IDL_tree tree);

typedef struct {
  OIDL_Marshal_Context *ctxt;
  enum { PI_BUILD_FUNC = 1<<0 } flags;
  OIDL_Marshal_Where where;
} OIDL_Populate_Info;

OIDL_Marshal_Node *marshal_populate(IDL_tree tree, OIDL_Marshal_Node *parent, OIDL_Populate_Info *pi);
#endif
