#ifndef ORBIT_IDL_C_BACKEND_H
#define ORBIT_IDL_C_BACKEND_H

#include "orbit-idl2.h"

#include <unistd.h>

typedef struct {
  char *base_name;
  FILE *fh;
} OIDL_C_Info;

void orbit_idl_output_c(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo);

/* Used internally */
void orbit_idl_output_c_headers(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_stubs(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_skeletons(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_common(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);
void orbit_idl_output_c_skelimpl(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo, OIDL_C_Info *ci);

/* utils */
void orbit_cbe_write_typespec(FILE *of, IDL_tree tree);
char * orbit_cbe_get_typename(IDL_tree tree);
void orbit_cbe_op_write_proto(FILE *of, IDL_tree op, const char *nom_prefix, gboolean for_epv);

#endif
