#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <orbit/orbit.h>

int
main (int argc, char **argv)
{
	gpointer p;
	int      i;

	p = ORBit_alloc_string (100);
	g_assert ((gulong)p & 0x1);
	for (i = 0; i < 100; i++)
		((guchar *)p) [i] = i;
	CORBA_free (p);

	p = CORBA_string_dup ("Foo");
	g_assert (((gulong)p & 0x1));
	CORBA_free (p);

	p = ORBit_alloc_simple (100);
	g_assert (!((gulong)p & 0x1));
	for (i = 0; i < 100; i++)
		((guchar *)p) [i] = i;
	CORBA_free (p);

	p = ORBit_alloc_tcval (TC_CORBA_sequence_CORBA_octet, 1);
	g_assert (!((gulong)p & 0x1));
	CORBA_free (p);

	p = ORBit_alloc_tcval (TC_ORBit_IInterface, 8);
	g_assert (!((gulong)p & 0x1));
	CORBA_free (p);

	return 0;
}
