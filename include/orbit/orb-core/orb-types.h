#ifndef ORB_TYPES_H
#define ORB_TYPES_H 1

/* The ALIGNOF_ values of basic types are computed by configure, and
 * appear in ORBit/config.h. We setup aligns for more complex types here.
 * TCVAL means CORBA_any._value, while ANY means the CORBA_any struct itself.
 * SEQ means the sequence struct, not its buffer.
 */
#define ALIGNOF_CORBA_ANY  MAX(		ALIGNOF_CORBA_STRUCT, 	\
					ALIGNOF_CORBA_POINTER)
#define ALIGNOF_CORBA_TCVAL MAX(MAX(	ALIGNOF_CORBA_LONG, 	\
					ALIGNOF_CORBA_STRUCT), 	\
					ALIGNOF_CORBA_POINTER)
#define ALIGNOF_CORBA_SEQ  MAX(MAX(	ALIGNOF_CORBA_STRUCT, 	\
					ALIGNOF_CORBA_LONG), 	\
					ALIGNOF_CORBA_POINTER)


typedef CORBA_char *CORBA_ORBid;
typedef struct _CORBA_any CORBA_any;

typedef struct {
        CORBA_unsigned_short _digits;
        CORBA_short _scale;
        signed char _sign;
        signed char _value[1];
} CORBA_fixed_d_s;

struct CORBA_NVList_type {
  CORBA_unsigned_long flags;	/* should be CORBA_Flags */
  GArray *list;
};

#define CORBA_OBJECT_NIL NULL

#define CORBA_ARG_IN (1<<0)
#define CORBA_ARG_OUT (1<<1)
#define CORBA_ARG_INOUT (1<<2)
#define CORBA_CTX_RESTRICT_SCOPE (1<<3)
#define CORBA_CTX_DELETE_DESCENDENTS (1<<4)
#define CORBA_OUT_LIST_MEMORY (1<<5)
#define CORBA_IN_COPY_VALUE (1<<6)
#define CORBA_DEPENDENT_LIST (1<<7)
#define CORBA_INV_NO_RESPONSE (1<<8)
#define CORBA_INV_TERM_ON_ERROR (1<<9)
#define CORBA_RESP_NO_WAIT (1<<10)

#endif
