#ifndef CORBA_ANY_H
#define CORBA_ANY_H 1

#define CORBA_any_set_release(a, r) (a)->_release = r
#define CORBA_any_get_release(a)    (a)->_release

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
void CORBA_any__copy(CORBA_any *out, const CORBA_any *in);
CORBA_any *CORBA_any__alloc(void);
#define CORBA_any_alloc CORBA_any__alloc
gpointer CORBA_any__freekids(gpointer mem, gpointer data);
CORBA_boolean ORBit_any_equivalent(CORBA_any *obj, CORBA_any *any, CORBA_Environment *ev);
CORBA_boolean ORBit_value_equivalent(gpointer *a, gpointer *b,
				     CORBA_TypeCode tc,
				     CORBA_Environment *ev);
gboolean ORBit_demarshal_value(CORBA_TypeCode tc,
			       gpointer *val,
			       GIOPRecvBuffer *buf,
			       gboolean dup_strings,
			       CORBA_ORB orb);
void ORBit_marshal_value(GIOPSendBuffer *buf,
			 gconstpointer *val,
			 CORBA_TypeCode tc,
			 ORBit_marshal_value_info *mi);
size_t ORBit_gather_alloc_info(CORBA_TypeCode tc);

#endif
