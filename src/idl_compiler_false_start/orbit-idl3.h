#ifndef ORBIT_IDL3_H
#define ORBIT_IDL3_H 1

#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <libIDL/IDL.h>
#include <orbit/util/basic_types.h>

#define _ORBIT_H_ /* kludge - we only want ORBIT_SERIAL */
#include <orbit/orbit.h>

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
} OIDL_Run_Info;

int orbit_idl_run(const char *filename, OIDL_Run_Info *rinfo);

#endif
