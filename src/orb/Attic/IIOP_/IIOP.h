#ifndef IIOP_H
#define IIOP_H 1

/* we are using the glib main loop, so tell people, who also use it
   not to use the IIOP{Add,Remove}ConnectionHandler */
#define ORBIT_USES_GLIB_MAIN_LOOP

#include <orbit/util/basic_types.h>
#include <IIOP/giop-connection.h>
#include <IIOP/iiop-connection.h>
#include <IIOP/giop-msg.h>
#include <IIOP/giop-types.h>
#include <IIOP/giop-msg-buffer.h>
#include <IIOP/giop-send-buffer.h>
#include <IIOP/giop-recv-buffer.h>

#endif /* IIOP_H */
