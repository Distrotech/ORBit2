#ifndef ORBIT_IDL3_H
#define ORBIT_IDL3_H 1

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <libIDL/IDL.h>
#include <orbit/util/basic_types.h>
#include <orbit/util/orbit-util.h>

#define _ORBIT_H_ /* kludge - we only want ORBIT_SERIAL */
#include <orbit/orbit.h>

#define ORBIT_RETVAL_VAR_NAME "_ORBIT_retval"

typedef struct {
  IDL_tree op1, op2;
} OIDL_Attr_Info;

typedef struct {
  char *cpp_args;

  char *output_formatter;

  char *output_language;
  char *input_filename;
  enum { OIDL_OUTPUT_STUBS=1<<0,
	 OIDL_OUTPUT_SKELS=1<<1,
	 OIDL_OUTPUT_COMMON=1<<2,
	 OIDL_OUTPUT_HEADERS=1<<3,
	 OIDL_OUTPUT_SKELIMPL=1<<4 } enabled_passes;

  int debug_level;
  int idl_warn_level;
  gboolean show_cpp_errors : 1;
  gboolean is_pidl : 1;
  gboolean do_skel_defs : 1;
  gboolean onlytop : 1;

  gpointer marshal_ctx;
} OIDL_Run_Info;

int orbit_idl_run(const char *filename, OIDL_Run_Info *rinfo);
typedef enum { DATA_IN=1, DATA_INOUT=2, DATA_OUT=4, DATA_RETURN=8 } IDL_ParamRole;

gint oidl_param_numptrs(IDL_tree param, IDL_ParamRole role);
gboolean orbit_cbe_type_is_fixed_length(IDL_tree ts);
IDL_tree orbit_cbe_get_typespec(IDL_tree node);
IDL_ParamRole oidl_attr_to_paramrole(enum IDL_param_attr attr);
void orbit_idl_print_node(IDL_tree node, int indent_level);

void orbit_idl_output_c(IDL_tree tree, OIDL_Run_Info *rinfo);

void IDL_tree_traverse_parents(IDL_tree p, GFunc f, gconstpointer func_data);
gint oidl_param_info(IDL_tree param, IDL_ParamRole role, gboolean *isSlice);
gboolean oidl_tree_is_pidl(IDL_tree tree);
void orbit_idl_attr_fake_ops(IDL_tree attr, IDL_ns ns);

#endif
