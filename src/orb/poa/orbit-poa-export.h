#ifndef ORBIT_POA_EXPORT_H
#define ORBIT_POA_EXPORT_H

/* Stuff that is internal to ORBit but not internal to the POA */
void ORBit_handle_request(CORBA_ORB orb, GIOPRecvBuffer *buf);

IOP_ObjectKey_info *ORBit_POA_object_to_okey(ORBit_POAObject *pobj);

#endif
