#ifndef ORBIT_IDL_MARSHAL_H
#define ORBIT_IDL_MARSHAL_H 1

#include "orbit-idl3.h"

typedef enum {
  MARSHAL_DATUM = 0,
  MARSHAL_LOOP = 1,
  MARSHAL_SWITCH = 2,
  MARSHAL_CASE = 3,
  MARSHAL_COMPLEX = 4,
  MARSHAL_CONST = 5,
  MARSHAL_SET = 6
} OIDL_Marshal_Node_Type;

typedef enum {
  MN_INOUT = 1<<0,
  MN_NSROOT = 1<<1,
  MN_NEED_TMPVAR = 1<<2,
  MN_NOMARSHAL = 1<<3,
  MN_ISSEQ = 1<<4,
  MN_ISSTRING = 1<<5,
  MN_LOOPED = 1<<6,
  MN_COALESCABLE = 1<<7,
  MN_ENDIAN_DEPENDANT = 1<<8,
  MN_DEMARSHAL_UPDATE_AFTER = 1<<9,
  MN_DEMARSHAL_CORBA_ALLOC = 1<<10,
  MN_DEMARSHAL_USER_MOD = 1<<11,
  MN_RECURSIVE_TOP = 1<<12
} OIDL_Marshal_Node_Flags;

typedef struct _OIDL_Marshal_Node OIDL_Marshal_Node;

struct _OIDL_Marshal_Node {
  OIDL_Marshal_Node *up;
  char *name;
  IDL_tree tree;
  OIDL_Marshal_Node_Type type;

  union {
    struct {
      OIDL_Marshal_Node *loop_var, *length_var;
      OIDL_Marshal_Node *contents;
    } loop_info;
    struct {
      OIDL_Marshal_Node *discrim;
      GSList *cases;
    } switch_info;
    struct {
      GSList *labels;
      OIDL_Marshal_Node *contents;
    } case_info;
    struct {
      guint32 datum_size;
    } datum_info;
    struct {
      OIDL_Marshal_Node *amount;
    } update_info;
    struct {
      guint32 amount;
    } const_info;
    struct {
      enum {
	CX_CORBA_FIXED,
	CX_CORBA_ANY, 
	CX_CORBA_OBJECT, 
	CX_CORBA_TYPECODE, 
	CX_CORBA_CONTEXT,
	CX_NATIVE
      } type;
      int context_item_count;
    } complex_info;
    struct {
      GSList *subnodes;
    } set_info;
  } u;
  OIDL_Marshal_Node_Flags flags;
  guint8 nptrs;
};

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
