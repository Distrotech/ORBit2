#include <config.h>
#include <orbit/poa/poa.h>

#define ORBit_return_bad_param_if_fail(expr)  G_STMT_START {		\
	if (!(expr)) {							\
		CORBA_exception_set_system (ev,				\
					    ex_CORBA_BAD_PARAM,		\
					    CORBA_COMPLETED_NO);	\
		return;							\
	}								\
} G_STMT_END

#define ORBit_return_val_bad_param_if_fail(expr, val)  G_STMT_START {	\
	if (!(expr)) {							\
		CORBA_exception_set_system (ev,				\
					    ex_CORBA_BAD_PARAM,		\
					    CORBA_COMPLETED_NO);	\
		return (val);						\
	}								\
} G_STMT_END

PortableServer_POA
PortableServer_ServantBase__default_POA (PortableServer_Servant  servant,
					 CORBA_Environment      *ev)
{
	ORBit_return_val_bad_param_if_fail (servant, CORBA_OBJECT_NIL);

	return ORBIT_SERVANT_TO_FIRST_POAOBJECT (servant)->poa;
}

CORBA_InterfaceDef
PortableServer_ServantBase__get_interface (PortableServer_Servant  servant,
					   CORBA_Environment      *ev)
{
	return CORBA_OBJECT_NIL;
}

CORBA_boolean
PortableServer_ServantBase__is_a (PortableServer_Servant  servant,
				  const CORBA_char       *logical_type_id,
				  CORBA_Environment      *ev)
{
	ORBit_return_val_bad_param_if_fail (servant, CORBA_FALSE);

	/*
	 * FIXME: actually implement this.
	 */

	return FALSE;
}

void
PortableServer_ServantBase__add_ref (PortableServer_Servant  servant,
				     CORBA_Environment      *ev)
{
}

void
PortableServer_ServantBase__remove_ref (PortableServer_Servant  servant,
					CORBA_Environment      *ev)
{
}

void
PortableServer_RefCountServantBase__add_ref (PortableServer_Servant  servant,
					     CORBA_Environment      *ev)
{
	ORBit_return_bad_param_if_fail (servant);

	/*
	 * FIXME: actually implement this.
	 */
}

void
PortableServer_RefCountServantBase__remove_ref (PortableServer_Servant  servant,
						CORBA_Environment      *ev)
{
	ORBit_return_bad_param_if_fail (servant);

	/*
	 * FIXME: actually implement this.
	 */
}

void
PortableServer_ServantBase__init (PortableServer_Servant  servant,
				  CORBA_Environment      *ev)
{
	PortableServer_ServantBase *servantbase;

	ORBit_return_bad_param_if_fail (servant);

	servantbase = (PortableServer_ServantBase *)servant;

	ORBit_return_bad_param_if_fail (servantbase->vepv && servantbase->vepv[0]);

	if (!servantbase->vepv[0]->finalize)
		servantbase->vepv[0]->finalize =
			PortableServer_ServantBase__fini;

	if (!servantbase->vepv[0]->default_POA)
		servantbase->vepv[0]->default_POA =
			PortableServer_ServantBase__default_POA;

	if (!servantbase->vepv[0]->get_interface)
		servantbase->vepv[0]->get_interface =
			PortableServer_ServantBase__get_interface;

	if (!servantbase->vepv[0]->is_a)
		servantbase->vepv[0]->is_a =
			PortableServer_ServantBase__is_a;

	if (!servantbase->vepv[0]->non_existent)
		servantbase->vepv[0]->add_ref =
			PortableServer_ServantBase__add_ref;

	if (!servantbase->vepv[0]->add_ref)
		servantbase->vepv[0]-> add_ref =
			PortableServer_ServantBase__add_ref;

	if (!servantbase->vepv[0]->remove_ref)
		servantbase->vepv[0]->remove_ref =
			PortableServer_ServantBase__remove_ref;
}

void
PortableServer_ServantBase__fini (PortableServer_Servant  servant,
				  CORBA_Environment      *ev)
{
	ORBit_return_bad_param_if_fail (servant);
}

void
PortableServer_RefCountServantBase__init (PortableServer_Servant  servant,
					  CORBA_Environment      *ev)
{
	PortableServer_ServantBase *servantbase;

	ORBit_return_bad_param_if_fail (servant);

	servantbase = (PortableServer_ServantBase *)servant;

	ORBit_return_bad_param_if_fail (servantbase->vepv && servantbase->vepv[0]);

	if (!servantbase->vepv[0]->finalize)
		servantbase->vepv[0]->finalize =
			PortableServer_RefCountServantBase__fini;

	if (!servantbase->vepv[0]->add_ref)
		servantbase->vepv[0]->add_ref = 
			PortableServer_RefCountServantBase__add_ref;

	if (!servantbase->vepv[0]->remove_ref)
		servantbase->vepv[0]->remove_ref =
			PortableServer_RefCountServantBase__remove_ref;

	PortableServer_ServantBase__init (servant, ev);
}

void
PortableServer_RefCountServantBase__fini (PortableServer_Servant  servant,
					  CORBA_Environment      *ev)
{
	ORBit_return_bad_param_if_fail (servant);

	PortableServer_RefCountServantBase__fini (servant, ev);
}
