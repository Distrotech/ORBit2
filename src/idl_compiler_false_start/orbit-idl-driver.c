#include "orbit-idl3.h"
#include <string.h>

int orbit_idl_run(const char *filename, OIDL_Run_Info *rinfo)
{
  IDL_ns namespace;
  IDL_tree tree;
  int errcode;

  errcode = IDL_parse_filename(filename, rinfo->cpp_args, NULL,
			       &tree, &namespace,
			       IDLF_TYPECODES|IDLF_CODEFRAGS,
			       IDL_WARNINGMAX);
  if(rinfo->debug_level > 1)
    orbit_idl_print_node(tree, 0);

  if(IDL_SUCCESS != errcode) {
    if(errcode == -1)
      g_warning("Parse of %s failed: %s", filename, g_strerror(errno));

    return errcode;
  }

  /* Do the work */
  orbit_idl_output_c(tree, rinfo);

  return 0;
}

