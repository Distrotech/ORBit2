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
  gpointer pass_info;
} OIDL_Output_Tree;

typedef struct {
  const char *name;
  void (*op_output)(OIDL_Output_Tree *tree, OIDL_Run_Info *rinfo);
} OIDL_Backend_Info;
OIDL_Backend_Info *orbit_idl_backend_for_lang(const char *lang);

#endif /* ORBIT_IDL2_H */
