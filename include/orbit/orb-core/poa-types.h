#ifndef POA_TYPES_H
#define POA_TYPES_H 1

typedef struct {
  PortableServer_Servant servant;
  PortableServer_ObjectId *oid;
  int *use_count;
  GFunc death_callback;
  gpointer user_data;
} ORBit_POAObject;

typedef struct {
} ORBit_POAInvokation;

typedef struct {
} POA_ORBit_ObjectImpl__epv;

#define ORBIT_STUB_GetPoaObj(x) (((CORBA_Object)x)->bypass_obj)
#define ORBIT_STUB_IsBypass(obj, classid) (((CORBA_Object)obj)->bypass_obj)
#define ORBIT_STUB_PreCall(x,y)
#define ORBIT_STUB_PostCall(x,y)
#define ORBIT_STUB_GetServant(x) NULL
#define ORBIT_STUB_GetEpv(x,y) NULL

#endif
