#ifndef __ORBIT_OPTIONS_H__
#define __ORBIT_OPTIONS_H__

#include <glib.h>

typedef enum {
	ORBIT_OPTION_NONE,
	ORBIT_OPTION_STRING,
	ORBIT_OPTION_INT,
	ORBIT_OPTION_BOOLEAN
} ORBit_option_type;

typedef struct {
	gchar             *name;
	ORBit_option_type  type;
	gpointer           arg;
} ORBit_option;

void ORBit_option_parse (int           *argc,
			 char         **argv,
			 ORBit_option  *options);

#endif /* __ORBIT_OPTIONS_H__ */
