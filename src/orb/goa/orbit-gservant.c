/*
 * orbit-gservant.c:
 *
 * Copyright (C) 2002 Sun Microsystems, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * Authors:
 *	Mark McLoughlin <mark@skynet.ie>
 *
 * Binary search array code adapted from glibs's GBSearchArray
 * which is Copyright (C) 2000-2001 Tim Janik, <timj@gtk.org>.
 */

#include <config.h>
#include <string.h>
#include <orbit/orbit.h>

#include "orbit-gservant.h"

#define ORBIT_GOA_OBJECT_TO_GSERVANT(obj) (((ORBitGServantPrivate *) (obj))->self)
#define ORBIT_GSERVANT_ADAPTOR(servant)   (((ORBit_OAObject) (servant)->priv)->adaptor)
#define ORBIT_GSERVANT_GOA(servant)       (((ORBit_GOA)((ORBit_OAObject) (servant)->priv)->adaptor))

struct _ORBitGServantPrivate {
	struct ORBit_OAObject_type  robj;

	ORBitGServant              *self;
	char                       *object_id;

	guint                       active : 1;
};

typedef struct {
	ORBitGServantSkeleton skeleton;
	gpointer              implementation;
} ORBitGServantClosure;

typedef struct {
	GType             itype;
	GQuark            opname_quark;
	guint             imethod_idx;
	ORBitGMethodFlags flags;
	guint             class_offset;
	gpointer          skeleton;
} ORBitGMethod;

enum {
	PROP_0,
	PROP_GOA,
};

static int ORBit_gmethod_compare (const ORBitGMethod *m1,
				  const ORBitGMethod *m2);

static GQuark          ORBit_gtype_data_quark = 0;
static ORBitGMethod   *ORBit_gmethod_bsa = NULL;
static guint           ORBit_gmethod_bsa_len = 0;
static GObjectClass   *parent_class = NULL;



void
ORBit_gtype_register_static (GType             itype,
			     ORBit_IInterface *iinterface)
{
	if (!ORBit_gtype_data_quark)
		ORBit_gtype_data_quark =
			g_quark_from_static_string ("ORBit-gtype-data");

	g_type_set_qdata (itype, ORBit_gtype_data_quark, iinterface);
}

static inline ORBit_IInterface *
ORBit_gtype_get_iinterface (GType itype)
{
	return g_type_get_qdata (itype, ORBit_gtype_data_quark);
}

static inline char *
ORBit_gtype_get_intf_id (GType itype)
{
	ORBit_IInterface *iinterface;

	iinterface = ORBit_gtype_get_iinterface (itype);

	return iinterface ? iinterface->tc->repo_id : NULL;
}

static inline GQuark
ORBit_gtype_get_intf_quark (GType itype)
{
	char *type_id;

	type_id = ORBit_gtype_get_intf_id (itype);

	return type_id ? g_quark_from_string (type_id) : 0;
}

static int
ORBit_gmethod_compare (const ORBitGMethod *m1,
		       const ORBitGMethod *m2)
{
#define BSEARCH_ARRAY_CMP(v1,v2) ((v1) < (v2) ? -1 : (v1) > (v2))

	if (m1->itype == m2->itype)
		return BSEARCH_ARRAY_CMP (m1->opname_quark, m2->opname_quark);
	else
		return BSEARCH_ARRAY_CMP (m1->itype, m2->itype);

#undef BSEARCH_ARRAY_CMP
}

static ORBitGMethod *
ORBit_gmethod_array_search (ORBitGMethod *key)
{
	ORBitGMethod *gmethods = ORBit_gmethod_bsa - 1;
	guint         n_gmethods = ORBit_gmethod_bsa_len;

	if (ORBit_gmethod_bsa_len == 0)
		return NULL;

	do {
		ORBitGMethod *check;
		guint         i;
		int           cmp;

		i = (n_gmethods + 1) / 2;
		check = gmethods + i;

		if ((cmp = ORBit_gmethod_compare (key, check)) == 0)
			return check;
		else if (cmp > 0) {
			n_gmethods -= i;
			gmethods = check;
		} else /* if (cmp < 0) */
			n_gmethods = i - 1;
	} while (n_gmethods);

	return NULL;
}

static void
ORBit_gmethod_array_insert (ORBitGMethod *gmethod)
{
	int i;

	if (!ORBit_gmethod_bsa)
		ORBit_gmethod_bsa = g_new (ORBitGMethod, 1);

	ORBit_gmethod_bsa_len++;
	ORBit_gmethod_bsa = g_renew (ORBitGMethod, ORBit_gmethod_bsa, ORBit_gmethod_bsa_len);

	for (i = 0; i < ORBit_gmethod_bsa_len - 1; i++)
		if (ORBit_gmethod_compare (&ORBit_gmethod_bsa [i], gmethod) > 0)
			break;

	g_memmove (ORBit_gmethod_bsa + i + 1,
		   ORBit_gmethod_bsa + i,
		   sizeof (ORBitGMethod) * (ORBit_gmethod_bsa_len - i - 1));

	*(ORBit_gmethod_bsa + i) = *gmethod;
}

static ORBitGMethod *
ORBit_gmethod_search (GQuark opname_quark,
		      GType  itype)
{
	ORBitGMethod key;

	key.opname_quark = opname_quark;
	key.itype        = itype;

	for (; key.itype; key.itype = g_type_parent (key.itype)) { /* FIXME: stop at ORBIT_TYPE_GSERVANT   */
		ORBitGMethod *method;                              /*        no point in continuing beyond */

		method = ORBit_gmethod_array_search (&key);
		if (method)
			return method;
	}

	return NULL;
}

void
ORBit_gmethod_register (const char        *method_name,
			GType              itype,
			guint              imethod_idx,
			ORBitGMethodFlags  flags,
			guint              class_offset,
			gpointer           skeleton)
{
	ORBitGMethod method, *existing;

	existing = ORBit_gmethod_search (g_quark_try_string (method_name), itype);
	if (existing) {
		g_warning (G_STRLOC ": method \"%s\" already exists in the \"%s\" class ancestry",
			   method_name,
			   g_type_name (itype) ? g_type_name (itype) : "<unknown>");
		return;
	}

	method.opname_quark = g_quark_from_static_string (method_name);
	method.itype        = itype;
	method.imethod_idx  = imethod_idx;
	method.flags        = flags;
	method.class_offset = class_offset;
	method.skeleton     = skeleton;

	ORBit_gmethod_array_insert (&method);
}



static void
ORBit_GOAObject_finalize (ORBit_RootObject robj)
{
	/* Nothing: we free this in ORBit_gservant_finalize */
}

static gboolean
ORBit_GOAObject_is_active (ORBit_OAObject adaptor_obj)
{
	g_return_val_if_fail (adaptor_obj != NULL, FALSE);
	g_return_val_if_fail (adaptor_obj->interface->adaptor_type == ORBIT_ADAPTOR_GOA, FALSE);

	g_assert (ORBIT_GOA_OBJECT_TO_GSERVANT (adaptor_obj)->priv->object_id != NULL);

	return TRUE;
}

static ORBit_ObjectKey *
ORBit_GOAObject_object_to_objkey (ORBit_OAObject adaptor_obj)
{
	ORBitGServant       *servant;
	ORBit_ObjectAdaptor  adaptor;
	ORBit_ObjectKey     *objkey;
	guchar              *mem;

	g_return_val_if_fail (adaptor_obj != NULL, NULL);
	g_return_val_if_fail (adaptor_obj->interface->adaptor_type == ORBIT_ADAPTOR_GOA, NULL);

	servant = ORBIT_GOA_OBJECT_TO_GSERVANT (adaptor_obj);

	g_assert (servant->priv->object_id != NULL);

        adaptor = adaptor_obj->adaptor;

        objkey           = CORBA_sequence_CORBA_octet__alloc ();
        objkey->_length  = adaptor->adaptor_key._length + ORBIT_GOA_OBJECT_ID_LEN;
        objkey->_maximum = objkey->_length;
        objkey->_buffer  = CORBA_sequence_CORBA_octet_allocbuf (objkey->_length);
        objkey->_release = CORBA_TRUE;

        mem = (guchar *) objkey->_buffer;
        memcpy (mem, adaptor->adaptor_key._buffer, adaptor->adaptor_key._length);

        mem += adaptor->adaptor_key._length;
        memcpy (mem, servant->priv->object_id, ORBIT_GOA_OBJECT_ID_LEN);

        return objkey;
}

static void 
ORBit_GOAObject_invoke (ORBit_OAObject     adaptor_obj,
			gpointer           ret,
			gpointer          *args,
			CORBA_Context      ctx,
			gpointer           data,
			CORBA_Environment *ev)
{
	ORBitGServantClosure *closure = (ORBitGServantClosure *) data;
	ORBitGServantPrivate *priv    = (ORBitGServantPrivate *) adaptor_obj;

	closure->skeleton (priv->self, ret, args, ctx,
			   ev, closure->implementation);
}

static void
ORBit_GOAObject_handle_request (ORBit_OAObject     adaptor_obj,
				CORBA_Identifier   opname,
				gpointer           ret,
				gpointer          *args,
				CORBA_Context      ctx,
				GIOPRecvBuffer    *recv_buffer,
				CORBA_Environment *ev)
{
	ORBitGServantPrivate *priv = (ORBitGServantPrivate *) adaptor_obj;

	ORBit_gservant_handle_request (priv->self, opname, ret,
				       args, ctx, recv_buffer, ev);
}

static ORBit_RootObject_Interface ORBit_GOAObject_interface = {
	ORBIT_ROT_OAOBJECT,
	ORBit_GOAObject_finalize
};

static struct
ORBit_OAObject_Interface_type ORBit_GOAObject_methods = {
	ORBIT_ADAPTOR_GOA, /* FIXME: needs to be added */
	(ORBitStateCheckFunc) ORBit_GOAObject_is_active,
	(ORBitKeyGenFunc)     ORBit_GOAObject_object_to_objkey,
	(ORBitInvokeFunc)     ORBit_GOAObject_invoke,
	(ORBitReqFunc)        ORBit_GOAObject_handle_request
};



static void
ORBit_gservant_finalize (GObject *object)
{
	ORBitGServant  *servant = ORBIT_GSERVANT (object);
	ORBit_OAObject  adaptor_obj;

	if (servant->priv->active)
		ORBit_gservant_deactivate (servant);

	if (ORBIT_GSERVANT_ADAPTOR (servant))
		ORBit_RootObject_release (ORBIT_GSERVANT_ADAPTOR (servant));

	adaptor_obj = &servant->priv->robj;
	if (adaptor_obj->objref)
		adaptor_obj->objref->adaptor_obj = NULL;

	g_free (servant->priv);
	servant->priv = NULL;

	parent_class->finalize (object);
}

static void
ORBit_gservant_set_property (GObject        *object,
			     guint           property_id,
			     const GValue   *value,
			     GParamSpec     *pspec)
{
	ORBitGServant *servant = ORBIT_GSERVANT (object);

	switch (property_id) {
	case PROP_GOA: {
		ORBit_GOA new_goa;

		if ((new_goa = g_value_get_pointer (value)))
			ORBit_RootObject_duplicate (new_goa);
		else
			new_goa = ORBit_goa_initial_reference (NULL, TRUE);

		if (ORBIT_GSERVANT_ADAPTOR (servant))
			ORBit_RootObject_release (ORBIT_GSERVANT_ADAPTOR (servant));

		ORBIT_GSERVANT_GOA (servant) = new_goa;
		}
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void
ORBit_gservant_get_property (GObject        *object,
			     guint           property_id,
			     GValue         *value,
			     GParamSpec     *pspec)
{
	ORBitGServant *servant = ORBIT_GSERVANT (object);

	switch (property_id) {
	case PROP_GOA:
		g_value_set_pointer (value, ORBIT_GSERVANT_ADAPTOR (servant));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
		break;
	}
}

static void
ORBit_gservant_is_a_skeleton (ORBitGServant     *servant,
			      gpointer           ret,
			      gpointer          *args,
			      gpointer           ctx,
			      CORBA_Environment *ev,
			      CORBA_boolean    (*is_a) (ORBitGServant     *servant,
							const CORBA_char  *repo_id,
							CORBA_Environment *ev))


{
        const char *type_id = *(const char **)args[0];

        *(CORBA_boolean *)ret = is_a (servant, type_id, ev);
}

static CORBA_boolean
ORBit_gservant_is_a (ORBitGServant     *servant,
		     const CORBA_char  *type_id,
		     CORBA_Environment *ev)
{
	return ORBit_IInterface_is_a (
			ORBit_gtype_get_iinterface (G_OBJECT_TYPE (servant)), type_id);
}

static void
ORBit_gservant_get_type_id_skeleton (ORBitGServant     *servant,
				     gpointer           ret,
				     gpointer          *args,
				     gpointer           ctx,
				     CORBA_Environment *ev,
				     CORBA_char        *(*get_type_id) (ORBitGServant     *servant,
									CORBA_Environment *ev))

{
        *(CORBA_char **)ret = get_type_id (servant, ev);
}

static CORBA_char *
ORBit_gservant_get_type_id (ORBitGServant     *servant,
			    CORBA_Environment *ev)
{
	return CORBA_string_dup (
			ORBit_gtype_get_intf_id (G_OBJECT_TYPE (servant)));
}

static void
ORBit_gservant_get_iinterface_skeleton (ORBitGServant     *servant,
					gpointer           ret,
					gpointer          *args,
					gpointer           ctx,
					CORBA_Environment *ev,
					ORBit_IInterface  *(*get_iinterface) (ORBitGServant     *servant,
									      const CORBA_char  *repo_id,
									      CORBA_Environment *ev))

{
        const char *repo_id = *(const char **)args[0];

        *(ORBit_IInterface **)ret = get_iinterface (servant, repo_id, ev);
}

/* cf. orb-core/orbit-typelib.c */
static ORBit_IInterface *
copy_iinterface (const ORBit_IInterface *idata, gboolean shallow)
{
        /* FIXME: we deep copy always for now - we should speed this up */
        /* FIXME: we need to set a flag here */
        return ORBit_copy_value (idata, TC_ORBit_IInterface);
}

ORBit_IInterface *
ORBit_gservant_get_iinterface (ORBitGServant     *servant,
			       const CORBA_char  *repo_id,
			       CORBA_Environment *ev)
{
	ORBit_IInterface *iinterface;

	iinterface = ORBit_gtype_get_iinterface (G_OBJECT_TYPE (servant));
	if (!iinterface || strcmp (repo_id, iinterface->tc->repo_id)) {
		CORBA_exception_set (
			ev, CORBA_USER_EXCEPTION,
			ex_ORBit_NoIInterface, NULL);
		return NULL;
	}

	return copy_iinterface (iinterface, TRUE);
}

static void
ORBit_gservant_class_init (ORBitGServantClass *klass,
			   gpointer            dummy)
{
	GObjectClass *gobject_class = (GObjectClass *) klass;

	parent_class = g_type_class_peek_parent (klass);

	gobject_class->finalize     = ORBit_gservant_finalize;
	gobject_class->set_property = ORBit_gservant_set_property;
	gobject_class->get_property = ORBit_gservant_get_property;

	g_object_class_install_property (
		gobject_class,
		PROP_GOA,
		g_param_spec_pointer ("goa",
				      "GOA",
				      "The GOA associated with the servant",
				      G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));

	klass->is_a           = ORBit_gservant_is_a;
	klass->get_type_id    = ORBit_gservant_get_type_id;
	klass->get_iinterface = ORBit_gservant_get_iinterface;

	ORBit_gmethod_register ("_is_a",
				G_OBJECT_CLASS_TYPE (klass),
				4, 0,
				G_STRUCT_OFFSET (ORBitGServantClass, is_a),
				ORBit_gservant_is_a_skeleton);

	ORBit_gmethod_register ("ORBit_get_type_id",
				G_OBJECT_CLASS_TYPE (klass),
				CORBA_OBJECT_SMALL_GET_TYPE_ID, 0,
				G_STRUCT_OFFSET (ORBitGServantClass, get_type_id),
				ORBit_gservant_get_type_id_skeleton);

	ORBit_gmethod_register ("ORBit_get_iinterface",
				G_OBJECT_CLASS_TYPE (klass),
				CORBA_OBJECT_SMALL_GET_IINTERFACE, 0,
				G_STRUCT_OFFSET (ORBitGServantClass, get_iinterface),
				ORBit_gservant_get_iinterface_skeleton);
}

static void
ORBit_gservant_instance_init (ORBitGServant      *servant,
			      ORBitGServantClass *klass)
{
	ORBit_RootObject robj;

	servant->priv = g_new0 (ORBitGServantPrivate, 1);

	servant->priv->self = servant;

	robj = (ORBit_RootObject) &servant->priv->robj;

	ORBit_RootObject_init (robj, &ORBit_GOAObject_interface);
	robj->refs = ORBIT_REFCOUNT_STATIC; /* Lifecycle controlled by GObject */

	((ORBit_OAObject) robj)->interface = &ORBit_GOAObject_methods;

	ORBIT_GSERVANT_GOA (servant) = ORBit_goa_initial_reference (NULL, TRUE);

	/* You may not create a ORBitGServant
	 * before initialising the ORB
	 */
	g_assert (ORBIT_GSERVANT_ADAPTOR (servant) != NULL);

	servant->priv->object_id = ORBit_goa_register_servant (
					ORBIT_GSERVANT_GOA (servant), servant);

	servant->priv->active = TRUE;
}

GType
ORBit_gservant_get_type (void)
{
	static GType type = 0;

	if (!type) {
		static const GTypeInfo info = {
			sizeof (ORBitGServantClass),
			NULL,
			NULL,
			(GClassInitFunc) ORBit_gservant_class_init,
			NULL,
			NULL,
			sizeof (ORBitGServant),
			0,
			(GInstanceInitFunc) ORBit_gservant_instance_init,
			NULL
		};

		type = g_type_register_static (
				G_TYPE_OBJECT, "ORBitGServant", &info, 0);

		ORBit_gtype_register_static (ORBIT_TYPE_GSERVANT, &CORBA_Object__iinterface);
	}

	return type;
}

/* ORBitGServant interface
 */
static ORBit_IMethod *
ORBit_gservant_lookup_method (ORBitGServant         *servant,
			      CORBA_Identifier       opname,
			      ORBitGServantSkeleton *skeleton,
			      gpointer              *implementation)
{
	ORBitGMethod     *method;
	ORBit_IInterface *iinterface;

	method = ORBit_gmethod_search (g_quark_try_string (opname), G_OBJECT_TYPE (servant));
	if (!method)
		return NULL;

	iinterface = ORBit_gtype_get_iinterface (method->itype);
	g_return_val_if_fail (method->imethod_idx < iinterface->methods._length, NULL);

	*skeleton       = method->skeleton;
	*implementation = G_STRUCT_MEMBER (gpointer,
					   G_OBJECT_GET_CLASS (servant),
					   method->class_offset);

	return &iinterface->methods._buffer [method->imethod_idx];
}

void
ORBit_gservant_handle_request (ORBitGServant     *servant,
			       CORBA_Identifier   opname,
			       gpointer           ret,
			       gpointer          *args,
			       CORBA_Context      ctx,
			       GIOPRecvBuffer    *recv_buffer,
			       CORBA_Environment *ev)
{
	ORBitGServantSkeleton  skeleton;
	ORBit_IMethod         *m_data;
	gpointer               implementation;

	/* The servant shouldn't be in the servant hash
	 * if it is not active, so the request should
	 * never get this far.
	 */
	g_assert (servant->priv->active);

	m_data = ORBit_gservant_lookup_method (
			servant, opname, &skeleton, &implementation);

	if (!m_data || !implementation) {
		if (!m_data)
			CORBA_exception_set_system (
				ev, ex_CORBA_BAD_OPERATION,
				CORBA_COMPLETED_NO);
		else
			CORBA_exception_set_system (
				ev, ex_CORBA_NO_IMPLEMENT,
				CORBA_COMPLETED_NO);


		if ((!m_data || !(m_data->flags & ORBit_I_METHOD_1_WAY)) && recv_buffer) {
			ORBit_recv_buffer_return_sys_exception (recv_buffer, ev);
			CORBA_exception_free (ev);
		}

		return;
	}

	if (recv_buffer) {
		ORBitGServantClosure closure;

		closure.implementation = implementation;
		closure.skeleton       = skeleton;

		ORBit_small_invoke_adaptor (
			&servant->priv->robj, recv_buffer,
			m_data, &closure, ev);
	} else
		skeleton (servant, ret, args, ctx, ev, implementation);
}

CORBA_Object
ORBit_gservant_to_reference (ORBitGServant *servant)
{
	ORBit_OAObject adaptor_obj;
	CORBA_Object   retval;
	GQuark         type_id;

	g_return_val_if_fail (ORBIT_IS_GSERVANT (servant), NULL);
	g_return_val_if_fail (servant->priv->active, NULL);

	adaptor_obj = &servant->priv->robj;

	if (adaptor_obj->objref)
		return ORBit_RootObject_duplicate (adaptor_obj->objref);

	type_id = ORBit_gtype_get_intf_quark (G_OBJECT_TYPE (servant));

	retval = adaptor_obj->objref =
			ORBit_objref_new (ORBIT_GSERVANT_ADAPTOR (servant)->orb, type_id);

	/* released by CORBA_Object_release */
	retval->adaptor_obj = ORBit_RootObject_duplicate (adaptor_obj);

        return ORBit_RootObject_duplicate (retval);
}

ORBitGServant *
ORBit_gservant_from_reference (CORBA_Object reference)
{
	ORBit_OAObject adaptor_obj;

	g_return_val_if_fail (reference != NULL, NULL);
	g_return_val_if_fail (reference->adaptor_obj != NULL, NULL);

	adaptor_obj = reference->adaptor_obj;
	g_return_val_if_fail (adaptor_obj->interface->adaptor_type == ORBIT_ADAPTOR_GOA, NULL);

	return g_object_ref (((ORBitGServantPrivate *) adaptor_obj)->self);
}

gboolean
ORBit_gservant_deactivate (ORBitGServant *servant)
{
	g_return_val_if_fail (ORBIT_IS_GSERVANT (servant), FALSE);
	g_return_val_if_fail (servant->priv->active, FALSE);

	if (servant->priv->object_id) {
		ORBit_goa_unregister_servant (
				ORBIT_GSERVANT_GOA (servant),
				servant->priv->object_id);

		g_free (servant->priv->object_id);
	}

	servant->priv->active = FALSE;

	return TRUE;
}
