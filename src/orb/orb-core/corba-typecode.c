#include "config.h"
#include <orbit/orbit.h>
#include <string.h>

typedef struct {
  CORBA_TypeCode tc;
  guint index; 
} TCRecursionNode;

typedef struct {
  GSList *prior_tcs;   /* Could be a hash table by typecode */
  guint   start_idx;
} TCEncodeContext;

typedef struct {
  GSList *prior_tcs;   /* Could be a hash table by offset */
  guint   current_idx; /* The offset from the start of the toplevel buffer of this buffer */
} TCDecodeContext;

#define get_wptr(buf) (buf->msg.header.message_size - ctx->start_idx)
#define get_rptr(buf) (buf->cur-buf->message_body)

static void
CDR_put_string(GIOPSendBuffer *c, const char *str)
{
  CORBA_unsigned_long len;
  len = strlen(str) + 1;
  giop_send_buffer_align(c, sizeof(len));
  giop_send_buffer_append_indirect(c, &len, sizeof(len));
  giop_send_buffer_append(c, str, len);
}

#define CDR_put_ulong(buf, n) (giop_send_buffer_align(buf, sizeof(n)), \
			       giop_send_buffer_append(buf, &(n), sizeof(n)))
#define CDR_put_long(buf, n) CDR_put_ulong(buf, n)
#define CDR_put_ulong_long(buf, n) CDR_put_ulong(buf, n)
#define CDR_put_wchar(buf, n) CDR_put_ulong(buf, n)
#define CDR_put_octet(buf, n) CDR_put_ulong(buf, n)
#define CDR_put_ushort(buf, n) CDR_put_ulong(buf, n)
#define CDR_put_short(buf, n) CDR_put_ulong(buf, n)

static void ORBit_TypeCode_free_fn(ORBit_RootObject obj_in);
static void tc_enc(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_objref(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_sequence(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_string(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_struct(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_union(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_enum(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_alias(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_except(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_array(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_fixed(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);
static void tc_enc_tk_wstring(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx);

static gboolean tc_dec(CORBA_TypeCode* t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_objref(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_sequence(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_string(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_struct(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_union(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_enum(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_alias(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_except(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_array(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_fixed(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);
static gboolean tc_dec_tk_wstring(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx);

typedef void (*CORBA_TypeCodeEncoder)(CORBA_TypeCode t,
				      GIOPSendBuffer *c,
				      TCEncodeContext* ctx);

typedef gboolean (*CORBA_TypeCodeDecoder)(CORBA_TypeCode t,
					  GIOPRecvBuffer *c,
					  TCDecodeContext* ctx);

typedef enum {
	TK_EMPTY,
	TK_SIMPLE,
	TK_COMPLEX
} TkType;

typedef struct {
	TkType type;
	CORBA_TypeCodeEncoder encoder;
	CORBA_TypeCodeDecoder decoder;
} TkInfo;

static const TkInfo tk_info[CORBA_tk_last]= {
	{TK_EMPTY, NULL, NULL}, /* tk_null */
	{TK_EMPTY, NULL, NULL}, /* tk_void */
	{TK_EMPTY, NULL, NULL}, /* tk_short */
	{TK_EMPTY, NULL, NULL}, /* tk_long */
	{TK_EMPTY, NULL, NULL}, /* tk_ushort */
	{TK_EMPTY, NULL, NULL}, /* tk_ulong */
	{TK_EMPTY, NULL, NULL}, /* tk_float */
	{TK_EMPTY, NULL, NULL}, /* tk_double */
	{TK_EMPTY, NULL, NULL}, /* tk_boolean */
	{TK_EMPTY, NULL, NULL}, /* tk_char */
	{TK_EMPTY, NULL, NULL}, /* tk_octet */
	{TK_EMPTY, NULL, NULL}, /* tk_any */
	{TK_EMPTY, NULL, NULL}, /* tk_TypeCode */
        {TK_EMPTY, NULL, NULL}, /* tk_Principal */
	{TK_COMPLEX, tc_enc_tk_objref, tc_dec_tk_objref}, /* tk_objref */
	{TK_COMPLEX, tc_enc_tk_struct, tc_dec_tk_struct}, /* tk_struct */
        {TK_COMPLEX, tc_enc_tk_union, tc_dec_tk_union}, /* tk_union */
        {TK_COMPLEX, tc_enc_tk_enum, tc_dec_tk_enum}, /* tk_enum */
        {TK_SIMPLE, tc_enc_tk_string, tc_dec_tk_string}, /* tk_string */
        {TK_COMPLEX, tc_enc_tk_sequence, tc_dec_tk_sequence}, /* tk_sequence */
        {TK_COMPLEX, tc_enc_tk_array, tc_dec_tk_array}, /* tk_array */
        {TK_COMPLEX, tc_enc_tk_alias, tc_dec_tk_alias}, /* tk_alias */
        {TK_COMPLEX, tc_enc_tk_except, tc_dec_tk_except}, /* tk_except */
        {TK_EMPTY, NULL, NULL}, /* tk_longlong */
        {TK_EMPTY, NULL, NULL}, /* tk_ulonglong */
        {TK_EMPTY, NULL, NULL}, /* tk_longdouble */
        {TK_EMPTY, NULL, NULL}, /* tk_wchar */
	{TK_SIMPLE, tc_enc_tk_wstring, tc_dec_tk_wstring}, /* tk_wstring */
	{TK_SIMPLE, tc_enc_tk_fixed, tc_dec_tk_fixed} /* tk_fixed */
};

const ORBit_RootObject_Interface ORBit_TypeCode_epv = {
  ORBIT_ROT_TYPECODE,
  ORBit_TypeCode_free_fn
};

#define DEF_TC_BASIC(nom) \
struct CORBA_TypeCode_struct TC_CORBA_##nom##_struct = { \
  {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC}, \
  CORBA_tk_##nom, \
  #nom, \
  "IDL:CORBA/" #nom ":1.0", \
  0, 0, NULL, NULL, NULL, CORBA_OBJECT_NIL, -1, 0, 0, 0 \
}

#define CORBA_tk_Object CORBA_tk_objref
#define CORBA_tk_unsigned_long CORBA_tk_ulong
#define CORBA_tk_long_long CORBA_tk_longlong
#define CORBA_tk_unsigned_long_long CORBA_tk_ulonglong
#define CORBA_tk_unsigned_short CORBA_tk_ushort
#define CORBA_tk_long_double CORBA_tk_longdouble

struct CORBA_TypeCode_struct TC_null_struct = {
  {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
  CORBA_tk_null,
  "null",
  "Null",
  0, 0, NULL, NULL, NULL, CORBA_OBJECT_NIL, -1, 0, 0, 0
};
struct CORBA_TypeCode_struct TC_void_struct = {
  {&ORBit_TypeCode_epv, ORBIT_REFCOUNT_STATIC},
  CORBA_tk_void,
  "void",
  "IDL:CORBA/void:1.0",
  0, 0, NULL, NULL, NULL, CORBA_OBJECT_NIL, -1, 0, 0, 0
};

DEF_TC_BASIC(char);
DEF_TC_BASIC(wchar);
DEF_TC_BASIC(string);
DEF_TC_BASIC(long);
DEF_TC_BASIC(unsigned_long);
DEF_TC_BASIC(float);
DEF_TC_BASIC(double);
DEF_TC_BASIC(short);
DEF_TC_BASIC(unsigned_short);
DEF_TC_BASIC(boolean);
DEF_TC_BASIC(octet);
DEF_TC_BASIC(any);
DEF_TC_BASIC(TypeCode);
DEF_TC_BASIC(Principal);
DEF_TC_BASIC(Object);
DEF_TC_BASIC(wstring);
DEF_TC_BASIC(long_double);
DEF_TC_BASIC(long_long);
DEF_TC_BASIC(unsigned_long_long);

gpointer
CORBA_TypeCode__freekids(gpointer mem, gpointer dat)
{
  CORBA_TypeCode t, *tp;

  tp = mem;
  t = *tp;

  CORBA_Object_release((CORBA_Object)t, NULL);

  return tp + 1;
}

void
ORBit_encode_CORBA_TypeCode(CORBA_TypeCode tc, GIOPSendBuffer* buf)
{
  TCEncodeContext ctx;
  GSList* l;

  ctx.start_idx = buf->msg.header.message_size;
  ctx.prior_tcs=NULL;
  tc_enc(tc, buf, &ctx);
  for(l=ctx.prior_tcs;l;l=l->next)
    g_free(l->data);
  g_slist_free(ctx.prior_tcs);
}

gboolean
ORBit_decode_CORBA_TypeCode(CORBA_TypeCode* tc, GIOPRecvBuffer* buf)
{
  TCDecodeContext ctx;
  GSList* l;
  gboolean retval;

  ctx.current_idx=0;
  ctx.prior_tcs=NULL;
  retval = tc_dec(tc, buf, &ctx);
  for(l=ctx.prior_tcs;l;l=l->next)
    g_free(l->data);
  g_slist_free(ctx.prior_tcs);
  return retval;
}

/* Encode a typecode to a codec, possibly recursively */

static void
tc_enc (CORBA_TypeCode   tc,
	GIOPSendBuffer  *c,
	TCEncodeContext *ctx)
{
  TCRecursionNode* node;
  const TkInfo* info;
  GSList* l;
  gint32 num, *tmpi;
  gint8 end;

  g_assert(CLAMP(0, tc->kind, CORBA_tk_last) == tc->kind);

  giop_send_buffer_align(c, sizeof(num));
  for(l=ctx->prior_tcs;l;l=l->next)
    {
      TCRecursionNode* node=l->data;
      /* CORBA_CORBA_TypeCode_equal might save space, but is slow.. */
      if(node->tc==tc)
	{
	  num = CORBA_tk_recursive;
	  giop_send_buffer_append_indirect(c, &num, sizeof(num));
	  num = node->index - c->msg.header.message_size - 4;
/*	  g_warning ("Offset = '%d' - '%d' = %d",
	  node->index, c->msg.header.message_size, num); */
	  giop_send_buffer_append_indirect(c, &num, sizeof(num));
	  return;
	}
    }

  node = g_new (TCRecursionNode, 1);
  node->tc = tc;
  node->index = c->msg.header.message_size;
  ctx->prior_tcs = g_slist_prepend (ctx->prior_tcs, node);

  giop_send_buffer_append(c, &tc->kind, sizeof(tc->kind));

	
  info=&tk_info[tc->kind];
  switch(info->type)
    {
    case TK_EMPTY:
      break;

    case TK_COMPLEX:
      giop_send_buffer_align(c, sizeof(num));
      num = 0;
      tmpi = (guint32*)giop_send_buffer_append_indirect(c, &num, sizeof(num));
      *tmpi = c->msg.header.message_size;
      end = GIOP_FLAG_ENDIANNESS;
      giop_send_buffer_append_indirect(c, &end, 1);
      (info->encoder)(tc, c, ctx);
      *tmpi = c->msg.header.message_size - *tmpi;
      break;
    case TK_SIMPLE:
      (info->encoder)(tc, c, ctx);
    }
}

static void
tc_enc_tk_objref(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  CDR_put_string(c, t->repo_id);
  CDR_put_string(c, t->name);
}

static void
tc_enc_tk_sequence(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  tc_enc(*t->subtypes, c, ctx);
  CDR_put_ulong(c, t->length);
}

static void
tc_enc_tk_string(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  CDR_put_ulong(c, t->length);
}

static void
tc_enc_tk_struct(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  CORBA_unsigned_long i;
  CDR_put_string(c, t->repo_id);
  CDR_put_string(c, t->name);
  CDR_put_ulong(c, t->sub_parts);
  for(i=0;i<t->sub_parts;i++)
    {
      CDR_put_string(c, t->subnames[i]);
      tc_enc(t->subtypes[i], c, ctx);
    }
}

/**
    Note that a union discriminator must of an integer (any size),
    octet, enum or char type. In particular, a discriminator
    type never embeds a pointer. So we can skip the whole CORBA_free
    mechanism and allocate simple memory for the types.
**/
static void
tc_enc_tk_union(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  CORBA_unsigned_long i;
  CDR_put_string(c, t->repo_id);
  CDR_put_string(c, t->name);
  tc_enc(t->discriminator, c, ctx);
  CDR_put_long(c, t->default_index);
  CDR_put_ulong(c, t->sub_parts);
  i=t->sub_parts;
  /* Thank goodness the discriminator types are rather limited,
     we can do the marshalling inline.. */
#define MEMBER_LOOPER_ENC(putname, typename, tkname) \
	case CORBA_tk_##tkname: \
		for(i=0;i<t->sub_parts;i++) \
                { \
			CDR_put_##putname(c, *(CORBA_##typename*) \
				      (t->sublabels[i]._value)); \
			CDR_put_string(c, t->subnames[i]); \
			tc_enc(t->subtypes[i], c, ctx); \
		} \
		break

#define UNION_MEMBERS(dir)					\
	MEMBER_LOOPER_##dir(ulong, long, long);			\
    case CORBA_tk_enum: /* fall through */			\
	MEMBER_LOOPER_##dir(ulong, unsigned_long, ulong);	\
	MEMBER_LOOPER_##dir(octet, boolean, boolean);		\
	MEMBER_LOOPER_##dir(octet, char, char);			\
	MEMBER_LOOPER_##dir(ushort, short, short);		\
	MEMBER_LOOPER_##dir(ushort, unsigned_short, ushort);	\
	MEMBER_LOOPER_##dir(ulong_long, long_long, longlong);	\
	MEMBER_LOOPER_##dir(ulong_long, unsigned_long_long, ulonglong);	\
	MEMBER_LOOPER_##dir(wchar, wchar, wchar);

  switch(t->discriminator->kind)
    {
      UNION_MEMBERS(ENC);
    default:
      g_error("tc_enc_tk_union: Illegal union discriminator "
	      "type %s\n", t->discriminator->name);
    }
}

static void
tc_enc_tk_enum(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  CORBA_unsigned_long i;
  CDR_put_string(c, t->repo_id);
  CDR_put_string(c, t->name);
  CDR_put_ulong(c, t->sub_parts);
  for(i=0;i<t->sub_parts;i++)
    CDR_put_string(c, t->subnames[i]);
}

static void
tc_enc_tk_alias(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  CDR_put_string(c, t->repo_id);
  CDR_put_string(c, t->name);
  tc_enc(*t->subtypes, c, ctx);
}

static void
tc_enc_tk_except(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  gulong i;
  CDR_put_string(c, t->repo_id);
  CDR_put_string(c, t->name);
  CDR_put_ulong(c, t->sub_parts);
  for(i=0;i<t->sub_parts;i++){
    CDR_put_string(c, t->subnames[i]);
    tc_enc(t->subtypes[i], c, ctx);
  }
}

static void
tc_enc_tk_array(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  tc_enc(*t->subtypes, c, ctx);
  CDR_put_ulong(c, t->length);
}

static void
tc_enc_tk_wstring(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  CDR_put_ulong(c, t->length);
}

static void
tc_enc_tk_fixed(CORBA_TypeCode t, GIOPSendBuffer *c, TCEncodeContext* ctx)
{
  CDR_put_ushort(c, t->digits);
  CDR_put_short(c, t->scale);
}

static void
ORBit_TypeCode_free_fn(ORBit_RootObject obj_in)
{
  CORBA_TypeCode tc = (CORBA_TypeCode)obj_in;
  int i;

  g_free((char*)(tc->name));
  g_free((char*)(tc->repo_id));

  for(i = 0; i < tc->sub_parts; i++)
    {
      if(tc->subnames)
	g_free((char*)(tc->subnames[i]));

      if(tc->subtypes)
	ORBit_RootObject_release(tc->subtypes[i]);

      if(tc->sublabels)
	CORBA_any__freekids(&tc->sublabels[i], NULL);
    }

  g_free(tc->subnames);
  g_free(tc->subtypes);
  g_free(tc->sublabels);

  if(tc->discriminator)
    ORBit_RootObject_release(tc->discriminator);

  g_free(tc);
}

static gboolean
CDR_get(GIOPRecvBuffer *buf, guchar *ptr, guint len, gboolean align)
{
  if(align)
    buf->cur = ALIGN_ADDRESS(buf->cur, len);
  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    return TRUE;
  memcpy(ptr, buf->cur, len);
  buf->cur += len;
  return FALSE;
}

#define _CDR_get(x, y) CDR_get(x, (guchar *)(y), sizeof(*(y)), TRUE)

#define CDR_get_ulong(x, y) _CDR_get(x, y)
#define CDR_get_ushort(x, y) _CDR_get(x, y)
#define CDR_get_short(x, y) _CDR_get(x, y)
#define CDR_get_ulong_long(x, y) _CDR_get(x, y)
#define CDR_get_octet(x, y) _CDR_get(x, y)
#define CDR_get_wchar(x, y) _CDR_get(x, y)

static gboolean
CDR_get_const_string(GIOPRecvBuffer *buf, char **ptr)
{
  CORBA_unsigned_long len;

  if(CDR_get_ulong(buf, &len))
    return TRUE;

  if((buf->cur + len) > buf->end
     || (buf->cur + len) < buf->cur)
    return TRUE;
  *ptr = g_memdup(buf->cur, len);
  buf->cur += len;

  return FALSE;
}

static gboolean
tc_dec(CORBA_TypeCode* t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  CORBA_TCKind kind;
  CORBA_unsigned_long lkind;
  CORBA_TypeCode tc;
  const TkInfo* info;
  TCRecursionNode* node;
  GIOPRecvBuffer *encaps;
  guint tmp_index;

  if(CDR_get_ulong(c, &lkind))
    return TRUE;

  kind = lkind;

  g_assert(CLAMP(0, kind, CORBA_tk_last) == kind);

  if(kind==CORBA_tk_recursive)
    {
      CORBA_long offset;
      GSList* l;
      if(CDR_get_ulong(c, &offset))
	return TRUE;
      for(l=ctx->prior_tcs;l;l=l->next)
	{
	  node=l->data;
	  if(offset == node->index - ctx->current_idx - get_rptr (c))
	    {
	      *t=ORBit_RootObject_duplicate (node->tc);
	      return FALSE;
	    }
/*	  else
            g_warning ("back tc mismactch '%d' == '%d - %d - %d' = '%d' Tk %d",
		       offset, node->index, ctx->current_idx, get_rptr (c),
		       node->index - ctx->current_idx - get_rptr (c),
		       node->tc->kind);*/
	}

      g_error("tc_dec: Invalid CORBA_TypeCode recursion offset "
	      "in input buffer\n");
      g_assert_not_reached();
    }

  g_assert(kind<CORBA_tk_last);

  node = g_new(TCRecursionNode, 1);
  node->index = ctx->current_idx + get_rptr(c) - 4; /* -4 for the TCKind */
  info=&tk_info[kind];
	
  tc = g_new0(struct CORBA_TypeCode_struct, 1);

  ORBit_RootObject_init(&tc->parent, &ORBit_TypeCode_epv);

  tc->kind=kind;
  switch(info->type)
    {

    case TK_EMPTY:
      break;

    case TK_COMPLEX:
      tmp_index=ctx->current_idx;
      ctx->current_idx += get_rptr (c) + 4;
      /* NB. the encaps buffer is for data validation */
      encaps = giop_recv_buffer_use_encaps_buf(c);
      (info->decoder)(tc, encaps, ctx);
      ctx->current_idx = tmp_index;
      giop_recv_buffer_unuse (encaps);
      break;
    case TK_SIMPLE:
      (info->decoder)(tc, c, ctx);
      break;
    }
  node->tc=tc;
  ctx->prior_tcs=g_slist_prepend(ctx->prior_tcs, node);
  *t=tc;

  return FALSE;
}

static gboolean
tc_dec_tk_objref(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  if(CDR_get_const_string(c, &t->repo_id))
    return TRUE;
  if(CDR_get_const_string(c, &t->name))
    return TRUE;
  return FALSE;
}

static gboolean
tc_dec_tk_sequence(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  t->subtypes=g_new0(CORBA_TypeCode, 1);
  if(tc_dec(&t->subtypes[0], c, ctx))
    return TRUE;
  ORBit_RootObject_duplicate(t->subtypes[0]);
  t->sub_parts = 1;
  if(CDR_get_ulong(c, &t->length))
    return TRUE;
  return FALSE;
}

static gboolean
tc_dec_tk_string(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  if(CDR_get_ulong(c, &t->length))
    return TRUE;
  return FALSE;
}
 
static gboolean
tc_dec_tk_struct(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  CORBA_unsigned_long i;

  if(CDR_get_const_string(c, &t->repo_id))
    return TRUE;
  if(CDR_get_const_string(c, &t->name))
    return TRUE;
  if(CDR_get_ulong(c, &t->sub_parts))
    return TRUE;
  t->subnames = g_new0(char*, t->sub_parts);
  t->subtypes = g_new0(CORBA_TypeCode, t->sub_parts);
  for(i=0;i<t->sub_parts;i++)
    {
      if(CDR_get_const_string(c, &t->subnames[i]))
	return TRUE;
      if(tc_dec(&t->subtypes[i], c, ctx))
	return TRUE;
      ORBit_RootObject_duplicate(t->subtypes[i]);
    }

  return FALSE;
}

static gboolean
tc_dec_tk_union(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  CORBA_unsigned_long i;
  guchar*	vec;
  long*	discrim_vec;

  if(CDR_get_const_string(c, &t->repo_id))
    return TRUE;
  if(CDR_get_const_string(c, &t->name))
    return TRUE;
  if(tc_dec(&t->discriminator, c, ctx))
    return TRUE;    
  ORBit_RootObject_duplicate(t->discriminator);
  if(CDR_get_ulong(c, &t->default_index))
    return TRUE;    
  if(CDR_get_ulong(c, &t->sub_parts))
    return TRUE;

	/* for simplicity, we assume discrim is longest possible size,
	 * and waste space. */
  vec = g_malloc0( (sizeof(CORBA_any) + sizeof(long)) * t->sub_parts);
  t->sublabels= (CORBA_any*) vec;
  discrim_vec = (long*) (vec + sizeof(CORBA_any) * t->sub_parts);
  t->subnames = g_new0(char*, t->sub_parts);
  t->subtypes = g_new0(CORBA_TypeCode, t->sub_parts);

#define MEMBER_LOOPER_DEC(getname, typename, tkname) \
    case CORBA_tk_##tkname: \
	for(i=0;i<t->sub_parts;i++){ 	\
	    t->sublabels[i]._type = 	\
	      ORBit_RootObject_duplicate(t->discriminator); \
	    t->sublabels[i]._value = &discrim_vec[i]; \
	    t->sublabels[i]._release = CORBA_FALSE; \
	    if(CDR_get_##getname(c, t->sublabels[i]._value)) \
               return TRUE; \
	    if(CDR_get_const_string(c, &t->subnames[i])) \
               return TRUE; \
	    if(tc_dec(&t->subtypes[i], c, ctx)) \
              return TRUE; \
	    ORBit_RootObject_duplicate(t->subtypes[i]); \
	} \
	break

  switch(t->discriminator->kind)
    {
      UNION_MEMBERS(DEC);
    default:
      /* XXX: what is correct error handling? */
      g_error("Don't know how to handle this type (%d) of discriminator.",
	      t->discriminator->kind);
    }

  return FALSE;
}

static gboolean
tc_dec_tk_enum(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  CORBA_unsigned_long i;
  if(CDR_get_const_string(c, &t->repo_id))
    return TRUE;
  if(CDR_get_const_string(c, &t->name))
    return TRUE;
  if(CDR_get_ulong(c, &t->sub_parts))
    return TRUE;

  t->subnames = g_new0(char*, t->sub_parts);
  for(i=0; i < t->sub_parts; i++)
    {
      if(CDR_get_const_string(c, &t->subnames[i]))
	return TRUE;
    }

  return FALSE;
}

static gboolean
tc_dec_tk_alias(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  CDR_get_const_string(c, &t->repo_id);
  CDR_get_const_string(c, &t->name);
  t->subtypes = g_new0(CORBA_TypeCode, 1);
  if(tc_dec(t->subtypes, c, ctx))
    return TRUE;
  ORBit_RootObject_duplicate(t->subtypes[0]);
  t->sub_parts = 1;
  return FALSE;
}


static gboolean
tc_dec_tk_except(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  gulong i;
  if(CDR_get_const_string(c, &t->repo_id))
    return TRUE;
  if(CDR_get_const_string(c, &t->name))
    return TRUE;
  if(CDR_get_ulong(c, &t->sub_parts))
    return TRUE;
  t->subtypes = g_new0(CORBA_TypeCode, t->sub_parts);
  t->subnames = g_new0(char *, t->sub_parts);
  for(i=0;i<t->sub_parts;i++)
    {
      if(CDR_get_const_string(c, &t->subnames[i]))
	return TRUE;
      if(tc_dec(&t->subtypes[i], c, ctx))
	return TRUE;
      ORBit_RootObject_duplicate(t->subtypes[i]);
    }
  return FALSE;
}

static gboolean
tc_dec_tk_array(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  t->subtypes=g_new(CORBA_TypeCode, 1);
  if(tc_dec(t->subtypes, c, ctx))
    return TRUE;

  ORBit_RootObject_duplicate(t->subtypes[0]);
  t->sub_parts = 1;
  if(CDR_get_ulong(c, &t->length))
    return TRUE;
  return FALSE;
}


static gboolean
tc_dec_tk_wstring(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  if(CDR_get_ulong(c, &t->length))
    return TRUE;
  return FALSE;
}

static gboolean
tc_dec_tk_fixed(CORBA_TypeCode t, GIOPRecvBuffer *c, TCDecodeContext* ctx)
{
  if(CDR_get_ushort(c, &t->digits))
    return TRUE;
  if(CDR_get_short(c, &t->scale))
    return TRUE;
  return FALSE;
}

/* FIXME: Right now this function doesn't record whether or not it has
   already visited a given TypeCode.  I'm not sure if every recursive
   type will have a tk_recursive node in it; if not, then this will
   need to be reworked a bit.  */
CORBA_boolean
CORBA_TypeCode_equal (CORBA_TypeCode obj,
		      CORBA_TypeCode tc,
		      CORBA_Environment *ev)
{
	int i;

	g_return_val_if_fail (tc != NULL, CORBA_FALSE);
	g_return_val_if_fail (obj != NULL, CORBA_FALSE);

	if (obj->kind != tc->kind)
		return CORBA_FALSE;

	switch (obj->kind) {
	case CORBA_tk_wstring:
	case CORBA_tk_string:
		return obj->length == tc->length;
	case CORBA_tk_objref:
		return ! strcmp (obj->repo_id, tc->repo_id);
	case CORBA_tk_except:
	case CORBA_tk_struct:
		if (strcmp (obj->repo_id, tc->repo_id)
		    || obj->sub_parts != tc->sub_parts)
			return CORBA_FALSE;
		for (i = 0; i < obj->sub_parts; ++i)
			if (! CORBA_TypeCode_equal (obj->subtypes[i],
						    tc->subtypes[i], ev))
				return CORBA_FALSE;
		break;
	case CORBA_tk_union:
		if (strcmp (obj->repo_id, tc->repo_id)
		    || obj->sub_parts != tc->sub_parts
		    || ! CORBA_TypeCode_equal (obj->discriminator,
					       tc->discriminator, ev)
		    || obj->default_index != tc->default_index)
			return CORBA_FALSE;
		for (i = 0; i < obj->sub_parts; ++i)

			if (! CORBA_TypeCode_equal (obj->subtypes[i],
						    tc->subtypes[i], ev)
			    || ! ORBit_any_equivalent (&obj->sublabels[i],
						       &tc->sublabels[i], ev))
				return CORBA_FALSE;

		break;
	case CORBA_tk_enum:
		if (obj->sub_parts != tc->sub_parts
		    || strcmp (obj->repo_id, tc->repo_id))
			return CORBA_FALSE;
		for (i = 0; i < obj->sub_parts; ++i)
			if (strcmp (obj->subnames[i], tc->subnames[i]))
				return CORBA_FALSE;
		break;
	case CORBA_tk_sequence:
	case CORBA_tk_array:
		if (obj->length != tc->length)
			return CORBA_FALSE;
		g_assert (obj->sub_parts == 1);
		g_assert (tc->sub_parts == 1);
		return CORBA_TypeCode_equal (obj->subtypes[0], tc->subtypes[0],
					     ev);
	case CORBA_tk_alias:
		if (strcmp (obj->repo_id, tc->repo_id))
			return CORBA_FALSE;
		
		g_assert (obj->sub_parts == 1);
		g_assert (tc->sub_parts == 1);

		return CORBA_TypeCode_equal (obj->subtypes[0], tc->subtypes[0],
					       ev);
		break;
	case CORBA_tk_recursive:
		return obj->recurse_depth == tc->recurse_depth;
	case CORBA_tk_fixed:
		return obj->digits == tc->digits && obj->scale == tc->scale;

	default:
		/* Everything else is primitive.  */
		break;
	}

	return CORBA_TRUE;
}
