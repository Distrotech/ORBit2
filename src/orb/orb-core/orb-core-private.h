#ifndef ORB_CORE_PRIVATE_H
#define ORB_CORE_PRIVATE_H 1

#include <orbit/orbit.h>

CORBA_TypeCode ORBit_get_union_tag(CORBA_TypeCode union_tc, gconstpointer *val,
				   gboolean update);
size_t ORBit_gather_alloc_info(CORBA_TypeCode tc);
gint ORBit_find_alignment(CORBA_TypeCode tc);
void ORBit_copy_value_core(gconstpointer *val, gpointer *newval, CORBA_TypeCode tc);

void ORBit_register_objref( CORBA_Object obj );
void ORBit_start_servers( CORBA_ORB orb );

/*
 * profile methods.
 */
void     IOP_generate_profiles( CORBA_Object obj );
void     IOP_register_profiles( CORBA_Object obj, GSList *profiles );
void     IOP_delete_profiles( GSList **profiles );

gboolean IOP_profile_get_info( CORBA_Object obj, gpointer *pinfo,
			       GIOPVersion *iiop_version, char **proto,
			       char **host, char **service, gboolean *ssl,
			       IOP_ObjectKey_info **oki, char *tmpbuf );

void     IOP_profile_hash( gpointer item, gpointer data );
gchar   *IOP_profile_dump( CORBA_Object obj, gpointer p );
gboolean IOP_profile_equal( CORBA_Object obj1, CORBA_Object obj2,
			    gpointer d1, gpointer d2 );
void     IOP_profile_marshal( CORBA_Object obj, GIOPSendBuffer *buf, gpointer *p );

gboolean ORBit_demarshal_IOR( CORBA_ORB orb, GIOPRecvBuffer *buf,
			      char **ret_type_id, GSList **ret_profiles);

#endif
