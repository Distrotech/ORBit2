#ifndef CORBA_ENVIRONMENT_H
#define CORBA_ENVIRONMENT_H 1

typedef struct {
  const CORBA_TypeCode tc;
  void (*marshal)(GIOPSendBuffer *_ORBIT_send_buffer, CORBA_Environment *ev);
} ORBit_exception_marshal_info;

typedef struct {
  const CORBA_TypeCode tc;
  void (*demarshal)(GIOPRecvBuffer *_ORBIT_recv_buffer, CORBA_Environment *ev);
} ORBit_exception_demarshal_info;

void CORBA_exception_set(CORBA_Environment *ev,
			 CORBA_exception_type major,
			 const CORBA_char *except_repos_id,
			 void *param);
CORBA_char *CORBA_exception_id(CORBA_Environment *ev);
void *CORBA_exception_value(CORBA_Environment *ev);
void CORBA_exception_init(CORBA_Environment *ev);
void CORBA_exception_free(CORBA_Environment *ev);
CORBA_any *CORBA_exception_as_any(CORBA_Environment *ev);
void CORBA_exception_set_system(CORBA_Environment *ev,
				const CORBA_char *except_repos_id,
				CORBA_completion_status completed);
void ORBit_handle_system_exception(CORBA_Environment *ev,
				   const CORBA_char *nom,
				   CORBA_completion_status status,
				   GIOPRecvBuffer *buf, GIOPSendBuffer *sendbuf);
void ORBit_handle_exception(GIOPRecvBuffer *buf, CORBA_Environment *ev,
			    const ORBit_exception_demarshal_info *ex_info,
			    CORBA_ORB orb);
GIOPConnection *ORBit_handle_location_forward(GIOPRecvBuffer *buf,
					      CORBA_Object obj);
void ORBit_send_system_exception(GIOPSendBuffer *buf, CORBA_Environment *ev);
void ORBit_send_user_exception(GIOPSendBuffer *send_buffer,
			       CORBA_Environment *ev,
			       const ORBit_exception_marshal_info *user_exceptions);

#endif
