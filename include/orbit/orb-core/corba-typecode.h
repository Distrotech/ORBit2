#ifndef CORBA_TYPECODE_H
#define CORBA_TYPECODE_H 1

#include <orbit/orb-core/corba-typecode-type.h>
#include <orbit/orb-core/corba-any-type.h>
#include <orbit/orb-core/orbit-object.h>

struct CORBA_TypeCode_struct {
	struct ORBit_RootObject_struct parent;
	CORBA_unsigned_long kind;
	const char *name;
	const char *repo_id;
	CORBA_unsigned_long length;
	CORBA_unsigned_long sub_parts;
	const char **subnames;		/* for struct, exception, union, enum */
	CORBA_TypeCode *subtypes;	/* for struct, exception, union, alias, array, sequence */
	CORBA_any *sublabels;		/* for union */
	CORBA_TypeCode discriminator;	/* for union */
	CORBA_unsigned_long recurse_depth; /* for recursive sequence */
	CORBA_long default_index;	/* for union */
	CORBA_unsigned_short digits; /* for fixed */
	CORBA_short scale;	     /* for fixed */
};

extern const struct CORBA_TypeCode_struct TC_CORBA_char_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_wchar_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_string_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_long_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_unsigned_long_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_short_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_unsigned_short_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_octet_struct;
#define TC_Object_struct TC_CORBA_Object_struct
extern const struct CORBA_TypeCode_struct TC_CORBA_Object_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_any_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_TypeCode_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_boolean_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_float_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_double_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_long_long_struct;
extern const struct CORBA_TypeCode_struct TC_CORBA_unsigned_long_long_struct;

extern const ORBit_RootObject_Interface ORBit_TypeCode_epv;

void ORBit_encode_CORBA_TypeCode(CORBA_TypeCode tc, GIOPSendBuffer* buf);
gboolean ORBit_decode_CORBA_TypeCode(CORBA_TypeCode* tc, GIOPRecvBuffer* buf);

#endif
