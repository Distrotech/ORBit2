#include <config.h>
#include <stdio.h>
#include <string.h>

#include <orbit/orbit.h>

static void
dump_tc (CORBA_TypeCode tc, int ident)
{
	char *id_str;

	id_str = g_new (char, ident + 1);
	memset (id_str, ' ', ident);
	id_str [ident] = '\0';

	printf ("%sType %12s: '%s'\n",
		id_str, TC_CORBA_TCKind->subnames [tc->kind],
		tc->repo_id);
}

static void
dump_iface (ORBit_IInterface *iface)
{
	int i;

	printf ("Interface '%s', %d methods\n",
		iface->tc->repo_id, iface->methods._length);

	for (i = 0; i < iface->base_interfaces._length; i++) {
		int j;

		printf ("  ");
		for (j = 0; j < i + 1; j++)
			printf ("  ");

		printf ("%s\n", iface->base_interfaces._buffer [i]);
	}

	printf ("\n");

	if (iface->methods._length > 0) {
		for (i = 0; i < iface->methods._length; i++) {
			ORBit_IMethod *m = &iface->methods._buffer [i];
			
			printf ("  %s (%d args, %s) %s%s\n",
				m->name, m->arguments._length,
				m->contexts._length ? "has context," : "",
				m->ret ? "returns " : "",
				m->ret ? m->ret->repo_id : "");
		}
	} else
		printf ("No methods\n");

	printf ("\n\n");
}

int
main (int argc, char *argv [])
{
	int                              i;
	const char                      *name;
	CORBA_sequence_CORBA_TypeCode   *tcs;
	CORBA_sequence_ORBit_IInterface *ifaces;

	name = argv [1];

	if (!ORBit_small_load_typelib (name))
		g_error ("Can't find typelib of name '%s' in path", name);

	tcs = ORBit_small_get_types (name);

	if (!tcs || tcs->_length == 0)
		printf ("No types\n");
	else {
		printf ("%d types:\n", tcs->_length);
		for (i = 0; i < tcs->_length; i++)
			dump_tc (tcs->_buffer [i], 0);
	}

	ifaces = ORBit_small_get_iinterfaces (name);
	if (!ifaces || ifaces->_length == 0)
		printf ("No IInterfaces\n");
	else {
		printf ("%d interfaces:\n", ifaces->_length);
		for (i = 0; i < ifaces->_length; i++)
			dump_iface (&ifaces->_buffer [i]);
	}

	return 0;
}
