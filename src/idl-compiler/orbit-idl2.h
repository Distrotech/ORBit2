#ifndef ORBIT_IDL2_H
#define ORBIT_IDL2_H 1

#include "orbit-idl3-types.h"
#include "orbit-idl-marshal.h"

struct _OIDL_Output_Tree {
  OIDL_Marshal_Context *ctxt;
  IDL_tree tree;
};

int orbit_idl_to_backend(const char *filename, OIDL_Run_Info *rinfo);

OIDL_Backend_Info *orbit_idl_backend_for_lang(const char *lang, const char *backend_dir);

/* genmarshal */
OIDL_Marshal_Node *orbit_idl_marshal_populate_in(IDL_tree tree, gboolean is_skels, OIDL_Marshal_Context *ctxt);
OIDL_Marshal_Node *orbit_idl_marshal_populate_out(IDL_tree tree, gboolean is_skels, OIDL_Marshal_Context *ctxt);
OIDL_Marshal_Node *orbit_idl_marshal_populate_except_marshal(IDL_tree tree, OIDL_Marshal_Context *ctxt);
OIDL_Marshal_Node *orbit_idl_marshal_populate_except_demarshal(IDL_tree tree, OIDL_Marshal_Context *ctxt);
char *oidl_marshal_node_fqn(OIDL_Marshal_Node *node);
char *oidl_marshal_node_valuestr(OIDL_Marshal_Node *node);

/* passes */
void orbit_idl_do_passes(IDL_tree tree, OIDL_Run_Info *rinfo);
void orbit_idl_do_node_passes(OIDL_Marshal_Node *node, gboolean is_out);

/* Utils */
void orbit_idl_attr_fake_ops(IDL_tree attr, IDL_ns ns);
void oidl_marshal_tree_dump(IDL_tree tree, int indent_level);
void oidl_marshal_node_dump(OIDL_Marshal_Node *tree, int indent_level);
void orbit_idl_print_node(IDL_tree node, int indent_level);
IDL_tree orbit_idl_get_array_type(IDL_tree tree);
char *orbit_idl_member_get_name(IDL_tree tree);
void orbit_idl_node_foreach(OIDL_Marshal_Node *node, GFunc func, gpointer user_data);
void IDL_tree_traverse_parents(IDL_tree p, GFunc f, gconstpointer func_data);
gboolean orbit_cbe_type_contains_complex(IDL_tree ts);

void orbit_idl_check_oneway_op (IDL_tree op);

typedef enum { DATA_IN=1, DATA_INOUT=2, DATA_OUT=4, DATA_RETURN=8 } IDL_ParamRole;
gint oidl_param_info(IDL_tree param, IDL_ParamRole role, gboolean *isSlice);

gint oidl_param_numptrs(IDL_tree param, IDL_ParamRole role);
gboolean orbit_cbe_type_is_fixed_length(IDL_tree ts);
IDL_tree orbit_cbe_get_typespec(IDL_tree node);
IDL_ParamRole oidl_attr_to_paramrole(enum IDL_param_attr attr);

extern gboolean oidl_tree_is_pidl(IDL_tree tree);


#define ORBIT_RETVAL_VAR_NAME "_ORBIT_retval"
#define ORBIT_EPV_VAR_NAME    "_ORBIT_epv"

#endif /* ORBIT_IDL2_H */
