#include "config.h"
#include <orbit/orbit.h>

void CORBA_free(gpointer mem)
{
}


gpointer
ORBit_alloc_core(size_t block_size,
		 ORBit_MemHow how,
		 size_t prefix_size,
		 gpointer *prefixref,
		 guint8 align)
{
  return NULL;
}

void
ORBit_free(gpointer mem)
{
  g_free(mem);
}


gpointer
ORBit_alloc_simple(size_t block_size)
{
  return NULL;
}

gpointer
ORBit_alloc_kidfnc(size_t element_size, guint num_elements,
		   ORBit_free_kidvals free_fnc)
{
  return NULL;
}
