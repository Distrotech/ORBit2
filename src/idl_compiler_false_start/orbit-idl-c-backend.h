#ifndef ORBIT_IDL_C_BACKEND_H
#define ORBIT_IDL_C_BACKEND_H

#include "orbit-idl3.h"

#include <unistd.h>

typedef struct {
  char *base_name, *c_base_name;
  FILE *fh;
  GString *ext_dcls;
  gboolean do_impl_hack;
  gboolean do_skel_defs;
} OIDL_C_Info;

/* Used internally */
void orbit_idl_output_c_headers(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_stubs(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_skeletons(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_common(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_skelimpl(IDL_tree tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);

void orbit_output_typecode(OIDL_C_Info *ci, IDL_tree ts);

/* utils */
char * orbit_cbe_get_typespec_str(IDL_tree tree);
void orbit_cbe_write_typespec(FILE *of, IDL_tree tree);
void orbit_cbe_write_param_typespec_raw(FILE *of, IDL_tree ts, IDL_ParamRole role);
void orbit_cbe_write_param_typespec(FILE *of, IDL_tree tree);
void orbit_cbe_op_write_proto(FILE *of, IDL_tree op, const char *nom_prefix, gboolean for_epv);
IDL_tree orbit_cbe_get_typespec(IDL_tree node);
void orbit_cbe_write_const(FILE *of, IDL_tree tree);
gboolean orbit_cbe_type_is_fixed_length(IDL_tree ts);
gboolean orbit_cbe_type_is_builtin(IDL_tree);
void orbit_cbe_param_printptrs(FILE *of, IDL_tree param, IDL_ParamRole role);
void orbit_cbe_id_define_hack(FILE *fh, const char *def_prefix, const char *def_name, const char *def_value);
void orbit_cbe_id_cond_hack(FILE *fh, const char *def_prefix, const char *def_name, const char *def_value);

#if 0
void orbit_cbe_write_const_node(FILE *of, OIDL_Marshal_Node *node);
void orbit_cbe_write_node_typespec(FILE *of, OIDL_Marshal_Node *node);
char *oidl_marshal_node_valuestr(OIDL_Marshal_Node *node);
void orbit_cbe_alloc_tmpvars(OIDL_Marshal_Node *node, OIDL_C_Info *ci);
#endif

#endif
