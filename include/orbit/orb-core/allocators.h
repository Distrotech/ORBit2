#ifndef ALLOCATORS_H
#define ALLOCATORS_H 1

#define PTR_TO_MEMINFO(x) (((ORBit_mem_info *)(x)) - 1)
#define MEMINFO_TO_PTR(x) ((gpointer)((x) + 1))

/**
    This function-type is used internally by the memory allocator
    as a callback.  When called, it must "free" anything contained
    in {mem}.  Normally, it does not free {mem} itself, only what is
    inside {mem}.  For example, if {mem} contains only integers, then
    nothing happens. Alternatively, if {mem} contains object references,
    then CORBA_Object_free (or ORBit_RootObject_release()) must be
    invoked on it.

    The callback func must return the "end" of {mem}. This is used
    when iterating through sequences, arrays and structs.

    Previously, older code supported the idea that the callback could
    return FALSE (or NULL?), in which case that meant that the callback
    itself had already free'd the memory. This convention is no longer
    used.

    Below, some magic values of the fnc ptr are defined.
**/
typedef gpointer (* ORBit_free_kidvals)(gpointer mem, gpointer func_data);
typedef void	 (* ORBit_free_blk)(gpointer mem, gpointer prefix);

#ifdef ORBIT_DEBUG
#define ORBIT_MAGIC_MEMPREFIX		0x1234fedc
#define	ORBIT_MEM_MAGICDEF(name)	gulong name 
#define ORBIT_MEM_MAGICSET(name)	(name) = ORBIT_MAGIC_MEMPREFIX
#else
#define ORBIT_MEM_MAGICDEF(name)
#define ORBIT_MEM_MAGICSET(name)
#endif

typedef gulong ORBit_MemHow;

#define ORBIT_MEMHOW_PRELEN_MASK	(0xF<<28)
#define ORBIT_MEMHOW_CODE_MASK		(0xF<<24)
#define ORBIT_MEMHOW_NUMELS_MASK	((1<<24)-1)

#define ORBIT_MEMHOW_NONE	(1<<24)
#define ORBIT_MEMHOW_SIMPLE	(2<<24)

#define ORBIT_MEMHOW_TYPECODE	(3<<24)
typedef struct ORBit_MemPrefix_TypeCode_type {
    ORBIT_MEM_MAGICDEF(magic);
    CORBA_TypeCode	tc;
} ORBit_MemPrefix_TypeCode;

#define ORBIT_MEMHOW_KIDFNC1	(4<<24)
typedef struct ORBit_MemPrefix_KidFnc1_type {
    ORBIT_MEM_MAGICDEF(magic);
    ORBit_free_kidvals	freekids;
} ORBit_MemPrefix_KidFnc1;

#define ORBIT_MEMHOW_FREEFNC1	(5<<24)
typedef struct ORBit_MemPrefix_FreeFnc1_type {
    ORBIT_MEM_MAGICDEF(magic);
    ORBit_free_blk	freeblk;
} ORBit_MemPrefix_FreeFnc1;


extern gpointer ORBit_alloc_core(size_t block_size,
				 ORBit_MemHow how,
				 size_t prefix_size,
				 gpointer *prefix_ref,
				 guint8 align);

extern void ORBit_free(gpointer mem);


gpointer ORBit_alloc_simple(size_t block_size);
gpointer ORBit_alloc_kidfnc(size_t element_size, guint num_elements,
				   ORBit_free_kidvals free_fnc);
gpointer ORBit_alloc_tcval(CORBA_TypeCode tc, guint nelements);

#define ORBit_alloc(sz, len, fnc) ORBit_alloc_kidfnc( (sz), (len), (fnc))

#define CORBA_sequence_set_release(s,r) (s)->_release = r

gpointer CORBA_sequence__freekids(gpointer mem, gpointer data);
gpointer CORBA_Object__freekids(gpointer mem, gpointer data);
gpointer CORBA_TypeCode__freekids(gpointer mem, gpointer data);
void CORBA_free(gpointer mem);

#endif
