#ifndef CORBA_ENVIRONMENT_H
#define CORBA_ENVIRONMENT_H 1

void CORBA_exception_set(CORBA_Environment *ev,
			 CORBA_exception_type major,
			 CORBA_char *except_repos_id,
			 void *param);
CORBA_char *CORBA_exception_id(CORBA_Environment *ev);
void *CORBA_exception_value(CORBA_Environment *ev);
void CORBA_exception_free(CORBA_Environment *ev);
CORBA_any *CORBA_exception_as_any(CORBA_Environment *ev);
void CORBA_exception_set_system(CORBA_Environment *ev,
				CORBA_char *except_repos_id,
				CORBA_completion_status completed);
void ORBit_handle_system_exception(CORBA_Environment *ev,
				   const CORBA_char *ex_name,
				   CORBA_completion_status status,
				   GIOPRecvBuffer *buf, GIOPSendBuffer *sendbuf);
void ORBit_handle_exception(GIOPRecvBuffer *buf, CORBA_Environment *ev,
			    gpointer ex_info, CORBA_ORB orb);
GIOPConnection *ORBit_handle_location_forward(GIOPRecvBuffer *buf,
					      CORBA_Object obj);

#endif
