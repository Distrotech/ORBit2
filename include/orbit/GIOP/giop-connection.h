#ifndef GIOP_CONNECTION_H
#define GIOP_CONNECTION_H 1

#include <orbit/GIOP/giop-types.h>
#include <linc/linc.h>
#include <orbit/GIOP/giop-server.h>
#include <netdb.h>

#define GIOP_TYPE_CONNECTION            (giop_connection_get_type ())
#define GIOP_TYPE_IS_CONNECTION(type)   (G_TYPE_FUNDAMENTAL (type) == GIOP_TYPE_CONNECTION)
#define GIOP_CONNECTION(object)	        (GIOP_IS_CONNECTION (object) ? ((GIOPConnection*) (object)) : \
				         G_TYPE_CHECK_INSTANCE_CAST ((object), GIOP_TYPE_CONNECTION, GIOPConnection))
#define GIOP_CONNECTION_CLASS(class)    (GIOP_IS_CONNECTION_CLASS (class) ? ((GIOPConnectionClass*) (class)) : \
				         G_TYPE_CHECK_CLASS_CAST ((class), GIOP_TYPE_CONNECTION, GIOPConnectionClass))
#define GIOP_IS_CONNECTION(object)      (((GIOPConnection*) (object)) != NULL && \
				         GIOP_IS_CONNECTION_CLASS (((GTypeInstance*) (object))->g_class))
#define GIOP_IS_CONNECTION_CLASS(class) (((GTypeClass*) (class)) != NULL && \
				         GIOP_TYPE_IS_CONNECTION (((GTypeClass*) (class))->g_type))

struct _GIOPConnection {
	LINCConnection  parent;

	GIOPRecvBuffer *incoming_msg;
	GMutex         *incoming_mutex;
	GMutex         *outgoing_mutex;

	GIOPVersion     giop_version;

	gpointer        orb_data;
};

typedef struct {
	LINCConnectionClass parent_class;
} GIOPConnectionClass;

GType giop_connection_get_type (void) G_GNUC_CONST;

GIOPConnection *giop_connection_initiate(const char *proto_name,
					 const char *remote_host_info,
					 const char *remote_serv_info,
					 LINCConnectionOptions options,
					 GIOPVersion giop_version);
void giop_connection_remove_by_orb (gpointer        match_orb_data);
void giop_connection_close         (GIOPConnection *cnx);

#endif
