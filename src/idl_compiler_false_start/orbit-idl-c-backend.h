#ifndef ORBIT_IDL_C_BACKEND_H
#define ORBIT_IDL_C_BACKEND_H

#include "orbit-idl2.h"

#include <unistd.h>

typedef struct {
  char *base_name, *c_base_name;
  FILE *fh;
} OIDL_C_Info;

void orbit_idl_output_c(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo);

/* Used internally */
void orbit_idl_output_c_headers(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_stubs(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_skeletons(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_common(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_skelimpl(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);

void orbit_output_typecode(OIDL_C_Info *ci, IDL_tree ts);

void c_marshal_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci);
void c_demarshal_generate(OIDL_Marshal_Node *node, OIDL_C_Info *ci);

/* utils */
typedef enum { DATA_IN=1, DATA_INOUT=2, DATA_OUT=4, DATA_RETURN=8 } IDL_ParamRole;
void orbit_cbe_write_typespec(FILE *of, IDL_tree tree);
char * orbit_cbe_get_typename(IDL_tree tree);
void orbit_cbe_op_write_proto(FILE *of, IDL_tree op, const char *nom_prefix, gboolean for_epv);
IDL_tree orbit_cbe_get_typespec(IDL_tree node);
void orbit_cbe_write_const(FILE *of, IDL_tree tree);
gboolean orbit_cbe_type_is_fixed_length(IDL_tree ts);

#endif
