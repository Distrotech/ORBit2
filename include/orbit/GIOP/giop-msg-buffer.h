#ifndef GIOP_MSG_BUFFER_H
#define GIOP_MSG_BUFFER_H 1

#include <orbit/IIOP/giop-types.h>
#include <orbit/IIOP/giop-connection.h>

typedef struct {
  GIOPConnection *cnx;
  GIOPMsgHeader header;
  GIOPMsg message;
} GIOPMsgBuffer;

#endif
