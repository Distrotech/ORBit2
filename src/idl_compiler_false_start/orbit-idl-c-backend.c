#include "config.h"

#include "orbit-idl-c-backend.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>

static FILE *out_for_pass(const char *input_filename, int pass, 
			  OIDL_Run_Info *rinfo);

void
orbit_idl_output_c(IDL_tree tree, OIDL_Run_Info *rinfo)
{
  int i;
  char *ctmp;
  OIDL_C_Info ci;

  ci.base_name = g_strdup(g_basename(rinfo->input_filename));
  ctmp = strrchr(ci.base_name, '.');
  g_assert(ctmp);
  *ctmp = '\0';

  ci.c_base_name = g_strdup(ci.base_name);
  if(!isalpha((guchar)ci.c_base_name[0]))
    ci.c_base_name[0] = '_';
  for(i = 0; ci.c_base_name[i]; i++) {
    if(!isalnum((guchar)ci.c_base_name[i])) ci.c_base_name[i] = '_';
  }

  ci.ext_dcls = g_string_new(0);

  ci.do_impl_hack = 1;
  ci.do_skel_defs = rinfo->do_skel_defs;
  for(i = 0; i < 5; i++) {
    if( (1 << i) & rinfo->enabled_passes) {
      ci.fh = out_for_pass(rinfo->input_filename, 1 << i, rinfo);
      
      switch(1 << i) {
      case OIDL_OUTPUT_STUBS:
	orbit_idl_output_c_stubs(tree, rinfo, &ci);
	break;
      case OIDL_OUTPUT_SKELS:
	orbit_idl_output_c_skeletons(tree, rinfo, &ci);
	break;
      case OIDL_OUTPUT_COMMON:
	orbit_idl_output_c_common(tree, rinfo, &ci);
	break;
      case OIDL_OUTPUT_HEADERS:
	orbit_idl_output_c_headers(tree, rinfo, &ci);
	break;
      case OIDL_OUTPUT_SKELIMPL:
	orbit_idl_output_c_skelimpl(tree, rinfo, &ci);
	break;
      }
      pclose(ci.fh);
    }
  }
  g_string_free(ci.ext_dcls,TRUE);
}

static FILE *
out_for_pass(const char *input_filename, int pass, OIDL_Run_Info *rinfo)
{
  char *tack_on = NULL; /* Quiet gcc */
  char *basein;
  char *ctmp;
  char *cmdline;

  basein = alloca(strlen(input_filename) + sizeof("-skelimpl.c"));
  strcpy(basein, g_basename(input_filename));

  ctmp = strrchr(basein, '.');

  g_assert(ctmp);

  *ctmp = '\0';

  switch(pass) {
  case OIDL_OUTPUT_STUBS:
    tack_on = "-stubs.c";
    break;
  case OIDL_OUTPUT_SKELS:
    tack_on = "-skels.c";
    break;
  case OIDL_OUTPUT_COMMON:
    tack_on = "-common.c";
    break;
  case OIDL_OUTPUT_HEADERS:
    tack_on = ".h";
    break;
  case OIDL_OUTPUT_SKELIMPL:
    tack_on = "-skelimpl.c";
    break;
  default:
    g_error("Unknown output pass");
    break;
  }

  strcat(basein, tack_on);

  cmdline = alloca(strlen(rinfo->output_formatter) + strlen(basein) 
		   + sizeof(" > "));
  sprintf(cmdline, "%s > %s", rinfo->output_formatter, basein);
  return popen(cmdline, "w");
}
