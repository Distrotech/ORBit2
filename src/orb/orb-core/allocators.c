#include "config.h"
#include <orbit/orbit.h>
#include "orb-core-private.h"

void CORBA_free(gpointer mem)
{
  ORBit_free(mem);
}


gpointer
ORBit_alloc_core(size_t block_size,
		 ORBit_MemHow how_in,
		 size_t prefix_size,
		 gpointer *prefix_ref,
		 guint8 align)
{
  size_t		psz;
  char		*pre, *mem;
  ORBit_MemHow	how;

  psz = prefix_size + sizeof(ORBit_MemHow);
  if ( align > 0 ) {
    if ( align < 4 )	/* MEMHOW must be aligned to 4 bytes */
      align = 4;
    psz = (psz+(align-1)) & ~(align-1);
  } else {
    psz = (psz+(ALIGNOF_CORBA_TCVAL-1)) & ~(ALIGNOF_CORBA_TCVAL-1);
  }
  pre = g_malloc(psz + block_size);
  /* NOTE: we assume g_malloc returns memory that meets the alignment
   * requirement. We're hosed if not!
   */
#ifdef ORBIT_DEBUG
  g_assert( (psz&3) == 0 );
#endif
  mem = pre + psz;
  how = how_in | ((psz>>2)<<28);
  *(ORBit_MemHow*)(mem - sizeof(ORBit_MemHow)) = how;
  *prefix_ref = pre;
  return mem;
}

void
ORBit_free_T(gpointer mem)
{
  ORBit_MemHow how, howcode, reps;
  gpointer     prefix, x;
  int          idx;

  if(!mem)
    return;

  how = * (ORBit_MemHow*) ( ((gchar*)(mem)) - sizeof(gulong) );
  if ( how==ORBIT_MEMHOW_NONE )
    return;

  prefix = ((gchar*)(mem)) - ((how>>28)<<2);
  howcode = how & ORBIT_MEMHOW_CODE_MASK;
  if ( howcode == ORBIT_MEMHOW_SIMPLE )
    {
      g_free(prefix);
      return;
    }

  reps = how & ORBIT_MEMHOW_NUMELS_MASK;
#ifdef ORBIT_DEBUG
  g_assert(*((gulong*)prefix) == ORBIT_MAGIC_MEMPREFIX);
  g_assert( reps != 0 );
#endif
  if ( howcode == ORBIT_MEMHOW_TYPECODE )
    {
      ORBit_MemPrefix_TypeCode	*pre = prefix;
      CORBA_TypeCode			tc = pre->tc;
      for (idx = 0, x = mem; idx < reps; idx++)
	x = ORBit_freekids_via_TypeCode_T(tc, x);
      ORBit_RootObject_release_T(tc);
      g_free(prefix);
      return;
    }

  if ( howcode == ORBIT_MEMHOW_KIDFNC1 )
    {
      ORBit_MemPrefix_KidFnc1		*pre = prefix;
      for (idx = 0, x = mem; idx < reps; idx++)
	x = pre->freekids(x, NULL);

      g_free(prefix);
      return;
    }

  if ( howcode == ORBIT_MEMHOW_FREEFNC1 ) {
    ORBit_MemPrefix_FreeFnc1	*pre = prefix;
    pre->freeblk(mem, pre);
    /* CB invokation must free any assoc. memory */
    return;
  }
  g_assert_not_reached();
}

void
ORBit_free(gpointer mem)
{
  O_MUTEX_LOCK(ORBit_RootObject_lifecycle_lock);

  ORBit_free_T (mem);

  O_MUTEX_UNLOCK(ORBit_RootObject_lifecycle_lock);
}

gpointer
ORBit_alloc_simple(size_t block_size)
{
  gpointer	mem, pre;
  if ( !block_size )
    return 0;
  mem = ORBit_alloc_core(block_size,
			 ORBIT_MEMHOW_SIMPLE, 0, &pre, /*align*/0);
  return mem;
}

gpointer
ORBit_alloc_kidfnc(size_t element_size, guint num_elements,
		   ORBit_free_kidvals free_fnc)
{
  gpointer			mem;
  ORBit_MemPrefix_KidFnc1	*pre;
  if ( !free_fnc)
    return ORBit_alloc_simple(element_size*num_elements);
  if ( !element_size || !num_elements )
    return 0;
  mem = ORBit_alloc_core(element_size*num_elements, 
			 ORBIT_MEMHOW_KIDFNC1|num_elements, sizeof(*pre), (gpointer)&pre,
			 /*align*/0);
  ORBIT_MEM_MAGICSET(pre->magic);
  pre->freekids = free_fnc;
  return mem;
}

gpointer
ORBit_alloc_tcval(CORBA_TypeCode tc, guint num_elements)
{
  size_t			element_size;
  ORBit_MemPrefix_TypeCode	*pre;
  gpointer 			mem;

  if ( num_elements==0
       || (element_size = ORBit_gather_alloc_info(tc)) == 0 ) {
    return 0;
  }
  mem = ORBit_alloc_core(element_size*num_elements,
			 ORBIT_MEMHOW_TYPECODE|num_elements, sizeof(*pre), (gpointer)&pre,
			 /*align*/0);
  ORBIT_MEM_MAGICSET(pre->magic);

  pre->tc = ORBit_RootObject_duplicate(tc);

  return mem;
}

/******************************************************************/

/**
    The argument {mem} is a chuck of memory described by {tc}, and its
    contents is freed, but {mem} itself is not freed. That is, if {mem}
    contains anything interesting (objrefs, pointers), they are freed.
    A pointer to the end of {mem} is returned. This should always be
    the same as {mem + ORBit_gather_alloc_info(tc)}. Also, any pointers
    within {mem} are zeroed; thus it should be safe to call this
    function multiple times on the same chunk of memory.

    This function is a modified version of ORBit_free_via_TypeCode().
    Aside from the arguments, the bigest difference is that this
    does not free the {tc} which is passed in. The old style led
    to a lot of pointless dups, and also failed miserably when
    arrays of things were allocated.
**/
gpointer
ORBit_freekids_via_TypeCode_T (CORBA_TypeCode tc,
			       gpointer       mem)
{
  int i;
  guchar *retval = NULL;
  CORBA_TypeCode	subtc;

/*  g_warning ("Freeing via tc '%s' at %p",
    ORBit_tk_to_name (tc->kind), mem);*/

  switch(tc->kind) {
  case CORBA_tk_any:
    {
      CORBA_any *pval = mem;
      if(pval->_release)
	ORBit_free_T(pval->_value);
      pval->_value = 0;
      ORBit_RootObject_release_T(pval->_type);
      pval->_type = 0;
      retval = (guchar *)(pval + 1);
    }
    break;
  case CORBA_tk_TypeCode:
  case CORBA_tk_objref:
    {
      CORBA_Object	*pval = mem;
      /*
       * FIXME: should we do this?
      ORBit_RootObject_release_T(*pval);
       */
      *pval = 0;
      retval = ((guchar *)mem) + sizeof(*pval);
    }
    break;
  case CORBA_tk_Principal:
    {
      CORBA_Principal *pval = mem;
      if(pval->_release)
	ORBit_free_T(pval->_buffer);
      pval->_buffer = 0;
      retval = (guchar *)(pval + 1);
    }
    break;
  case CORBA_tk_except:
  case CORBA_tk_struct:
    for(i = 0; i < tc->sub_parts; i++) {
      subtc = tc->subtypes[i];
      mem = ALIGN_ADDRESS(mem, ORBit_find_alignment(subtc));
      mem = ORBit_freekids_via_TypeCode_T(subtc, mem);
    }
    retval = mem;
    break;
  case CORBA_tk_union:
    {
      int sz = 0;
      int al = 1;
      gconstpointer cmem = mem;
      subtc = ORBit_get_union_tag(tc, &cmem, TRUE);
      for(i = 0; i < tc->sub_parts; i++) {
	al = MAX(al, ORBit_find_alignment(tc->subtypes[i]));
	sz = MAX(sz, ORBit_gather_alloc_info(tc->subtypes[i]));
      }
      mem = ALIGN_ADDRESS(cmem, al);
      ORBit_freekids_via_TypeCode_T(subtc, mem);
      /* the end of the body (subtc) may not be the
       * same as the end of the union */
      retval = ((guchar *)mem) + sz;
    }
    break;
  case CORBA_tk_wstring:
  case CORBA_tk_string:
    {
      CORBA_char **pval = mem;
      ORBit_free_T(*pval);
      *pval = 0;
      retval = (guchar *)mem + sizeof(*pval);
    }
    break;
  case CORBA_tk_sequence:
    {
      CORBA_sequence_CORBA_octet *pval = mem;
      if(pval->_release)
	ORBit_free_T(pval->_buffer);
      pval->_buffer = 0;
      retval = (guchar *)mem + sizeof(*pval);
    }
    break;
  case CORBA_tk_array:
    for(i = 0; i < tc->length; i++) {
      mem = ORBit_freekids_via_TypeCode_T(tc->subtypes[0], mem);
    }
    retval = mem;
    break;
  case CORBA_tk_alias:
    retval = ORBit_freekids_via_TypeCode_T(tc->subtypes[0], mem);
    break;
  default:
    {
      gulong length, align;
      length = ORBit_gather_alloc_info (tc);
      align  = ORBit_find_alignment (tc);
      retval = ALIGN_ADDRESS(mem, align) + length;
    }
    break;
  }
  return (gpointer)retval;
}

gpointer
ORBit_freekids_via_TypeCode(CORBA_TypeCode tc, gpointer mem)
{
  gpointer ret;

  O_MUTEX_LOCK(ORBit_RootObject_lifecycle_lock);

  ret = ORBit_freekids_via_TypeCode_T (tc, mem);

  O_MUTEX_UNLOCK(ORBit_RootObject_lifecycle_lock);

  return ret;
}
