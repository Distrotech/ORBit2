
/*
 * from main program
 */
extern gboolean		echo_opt_quiet;
/*
 * from echo-srv.c 
 */
extern void		echo_srv_start_poa(CORBA_ORB orb,CORBA_Environment *ev);
extern CORBA_Object	echo_srv_start_object(CORBA_Environment *ev);
extern void		echo_srv_finish_object(CORBA_Environment *ev);
extern void		echo_srv_finish_poa(CORBA_Environment *ev);
