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

#endif
