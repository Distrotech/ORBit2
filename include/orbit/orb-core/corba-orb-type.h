#ifndef CORBA_ORB_TYPE
#define CORBA_ORB_TYPE 1

#include <orbit/orb-core/orb-types.h>
#include <orbit/orb-core/orbit-object.h>

#if !defined(ORBIT_DECL_CORBA_ORB) && !defined(_CORBA_ORB_defined)
#define ORBIT_DECL_CORBA_ORB 1
#define _CORBA_ORB_defined 1
typedef struct CORBA_ORB_type *CORBA_ORB;
#endif

#if !defined(_CORBA_ORB_ObjectId_defined)
#define _CORBA_ORB_ObjectId_defined 1
   typedef CORBA_string CORBA_ORB_ObjectId;
#define CORBA_ORB_ObjectId_marshal(x,y,z) CORBA_string_marshal((x),(y),(z))
#define CORBA_ORB_ObjectId_demarshal(x,y,z,i) CORBA_string_demarshal((x),(y),(z),(i))
#if !defined(TC_IMPL_TC_CORBA_ORB_ObjectId_0)
#define TC_IMPL_TC_CORBA_ORB_ObjectId_0 'c'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_1 'o'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_2 'r'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_3 'b'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_4 'a'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_5 '_'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_6 'd'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_7 'e'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_8 'f'
#define TC_IMPL_TC_CORBA_ORB_ObjectId_9 's'
   extern const struct CORBA_TypeCode_struct TC_CORBA_ORB_ObjectId_struct;
#define TC_CORBA_ORB_ObjectId ((CORBA_TypeCode)&TC_CORBA_ORB_ObjectId_struct)
#endif
#define CORBA_ORB_ObjectId__freekids CORBA_string__freekids
#endif

struct CORBA_ORB_type {
  struct ORBit_RootObject_struct root_object;
  GIOPVersion default_giop_version;

  GSList *servers;
  GPtrArray *poas;
  gpointer poa_current;
  gpointer poa_current_invocations;
  GHashTable *initial_refs;
  ORBit_genrand genrand;
  guint life_flags;
};

typedef struct {
  gpointer objref;
  gboolean listme : 1;
  gboolean free_name : 1; /* Might as well use up the space since it's here */
} ORBit_InitialReference;

#endif
