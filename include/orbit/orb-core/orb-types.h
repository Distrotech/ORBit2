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

#define CORBA_OBJECT_NIL NULL

#endif
