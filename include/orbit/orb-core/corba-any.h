#ifndef CORBA_ANY_H
#define CORBA_ANY_H 1

void ORBit_marshal_arg(GIOPSendBuffer *buf,
                       gconstpointer val,
                       CORBA_TypeCode tc);
void ORBit_marshal_any(GIOPSendBuffer *buf, const CORBA_any *val);
gpointer ORBit_demarshal_arg(GIOPRecvBuffer *buf,
                             CORBA_TypeCode tc,
                             gboolean dup_strings,
                             CORBA_ORB orb);
gboolean ORBit_demarshal_any(GIOPRecvBuffer *buf, CORBA_any *retval,
			     gboolean dup_strings,
			     CORBA_ORB orb);
gboolean ORBit_demarshal_value(CORBA_TypeCode tc,
			       gpointer *val,
			       GIOPRecvBuffer *buf,
			       gboolean dup_strings,
			       CORBA_ORB orb);
gpointer ORBit_copy_value(gconstpointer value, CORBA_TypeCode tc);
void CORBA_any__copy(CORBA_any *out, CORBA_any *in);
void CORBA_any_set_release(CORBA_any *val, CORBA_boolean setme);
CORBA_any *CORBA_any__alloc(void);
#define CORBA_any_alloc CORBA_any__alloc
gpointer CORBA_any__freekids(gpointer mem, gpointer data);

#endif
