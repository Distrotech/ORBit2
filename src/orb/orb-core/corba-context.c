#include "config.h"
#include <orbit/orbit.h>

void
CORBA_Context_set_one_value(CORBA_Context _obj,
			    const CORBA_Identifier prop_name,
				    const CORBA_char * value,
				    CORBA_Environment * ev)
{
}

void
CORBA_Context_set_values(CORBA_Context _obj,
			      const CORBA_NVList values,
			      CORBA_Environment * ev)
{
}

void
CORBA_Context_get_values(CORBA_Context _obj,
			      const CORBA_Identifier start_scope,
			      const CORBA_Flags op_flags,
			      const CORBA_Identifier prop_name,
			      CORBA_NVList * values,
			      CORBA_Environment * ev)
{
}

void
CORBA_Context_delete_values(CORBA_Context _obj,
				 const CORBA_Identifier prop_name,
				 CORBA_Environment * ev)
{
}

void
CORBA_Context_create_child(CORBA_Context _obj,
				const CORBA_Identifier ctx_name,
				CORBA_Context * child_ctx,
				CORBA_Environment * ev)
{
}

void
CORBA_Context_delete(CORBA_Context _obj, const CORBA_Flags del_flags,
			  CORBA_Environment * ev)
{
}
