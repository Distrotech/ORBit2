#ifndef ORBIT_IDL2_H
#define ORBIT_IDL2_H 1

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <libIDL/IDL.h>

typedef struct {
  char *cpp_args;
  int debug_level;

  enum { OUTPUT_STUBS=1<<0,
	 OUTPUT_SKELS=1<<1,
	 OUTPUT_COMMON=1<<2,
	 OUTPUT_HEADERS=8,
	 OUTPUT_SKELIMPL=16 } enabled_passes;

  char *output_formatter;

  char *output_language;
  char *input_filename;
} OIDL_Run_Info;

int orbit_idl_to_backend(const char *filename, OIDL_Run_Info *rinfo);

typedef struct {
  IDL_tree tree;
} OIDL_Output_Tree;

/* yadda yadda:
   Fixed length as in bulk-marshallable
   Fixed length as in a terminal allocation.

   Turn an IDL_LIST of params into a tree of marshalling info.
     Each node will need to give:
         Type (datum, loop, switch, string, complex)
	 Name
	 Subnodes (loop & switch only)
	 Dependencies

   Note for string_info.length_var, loop_info.loop_var, switch_info.discrim - these are all subnodes of the current node,
   not pointers to other unrelated nodes.

   dependencies is a list of pointers to unrelated nodes.
 */
typedef struct _OIDL_Marshal_Node OIDL_Marshal_Node;

typedef enum { MARSHAL_DATUM, MARSHAL_LOOP, MARSHAL_SWITCH,
	       MARSHAL_COMPLEX, MARSHAL_UPDATE, MARSHAL_CONST, MARSHAL_SET,
	       MARSHAL_ALLOCATE } OIDL_Marshal_Node_Type;
typedef enum {
  MN_POINTER_VAR,
  MN_INOUT /* Needs freeing before alloc */
} OIDL_Marshal_Node_Flags;

struct _OIDL_Marshal_Node {
  OIDL_Marshal_Node *up;
  int nrefs;
  char *name;
  IDL_tree tree;
  OIDL_Marshal_Node_Type type;
  GSList *dependencies;
  union {
    struct {
      OIDL_Marshal_Node *loop_var, *length_var;
      OIDL_Marshal_Node *contents;
      guint is_string : 1;
    } loop_info;
    struct {
      OIDL_Marshal_Node *discrim, *contents;
    } switch_info;
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
    } complex_info;
    struct {
      GSList *subnodes;
    } set_info;
    struct {
    } allocate_info;
  } u;
  OIDL_Marshal_Node_Flags flags;
};

/* Handling an IDLN_ATTR_DCL:
   foreach(node->simple_declarations) {
       turn node->data into a OIDL_Attr_Info.
       Process op1 & op2.
   }
*/
typedef struct {
  IDL_tree op1, op2;
} OIDL_Attr_Info;
typedef struct {
  OIDL_Marshal_Node *in, *out;
} OIDL_Op_Info;

typedef struct {
  const char *name;
  void (*op_output)(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo);
} OIDL_Backend_Info;
OIDL_Backend_Info *orbit_idl_backend_for_lang(const char *lang);

/* genmarshal */
OIDL_Marshal_Node *orbit_idl_marshal_populate_in(IDL_tree tree);
OIDL_Marshal_Node *orbit_idl_marshal_populate_out(IDL_tree tree);
gboolean orbit_idl_marshal_endian_dependant_p(OIDL_Marshal_Node *node);

/* passes */
void orbit_idl_do_passes(IDL_tree tree);

/* Utils */
void orbit_idl_attr_fake_ops(IDL_tree attr);

#define ORBIT_RETVAL_VAR_NAME "_ORBIT_retval"

#endif /* ORBIT_IDL2_H */
