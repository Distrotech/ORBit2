#ifndef CORBA_OBJECT_TYPE_H
#define CORBA_OBJECT_TYPE_H 1

#include <glib.h>

G_BEGIN_DECLS

#if defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API)

#ifndef ORBIT2_INTERNAL_API
#define  GIOPConnection void
#endif

typedef CORBA_sequence_CORBA_octet ORBit_ObjectKey;

struct CORBA_Object_type {
	/* None of these fields are used by the stubs */
	struct ORBit_RootObject_struct parent;

	GIOPConnection                *connection;        /* l */
	GQuark                         type_qid;
	GSList                        *profile_list;      /* l */
	GSList                        *forward_locations; /* l */
	ORBit_ObjectKey               *object_key;        /* l */

	CORBA_ORB                      orb;         /* possibly used by stubs */
	ORBit_OAObject                 adaptor_obj; /* used by stubs */
};

#endif /* defined(ORBIT2_INTERNAL_API) || defined (ORBIT2_STUBS_API) */

G_END_DECLS

#endif
