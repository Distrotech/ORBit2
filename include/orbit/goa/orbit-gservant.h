/*
 * orbit-gservant.h:
 *
 * Copyright (C) 2001 Sun Microsystems, Inc.
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
 */

#ifndef __ORBIT_GSERVANT_H__
#define __ORBIT_GSERVANT_H__

#include <glib/gmacros.h>
#include <glib-object.h>
#include <orbit/orbit.h>

G_BEGIN_DECLS

#define ORBIT_TYPE_GSERVANT         (ORBit_gservant_get_type ())
#define ORBIT_GSERVANT(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), ORBIT_TYPE_GSERVANT, ORBitGServant))
#define ORBIT_GSERVANT_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), ORBIT_TYPE_GSERVANT, ORBitGServantClass))
#define ORBIT_IS_GSERVANT(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), ORBIT_TYPE_GSERVANT))
#define ORBIT_IS_GSERVANT_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), ORBIT_TYPE_GSERVANT))
#define ORBIT_GSERVANT_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), ORBIT_TYPE_GSERVANT, ORBitGServantClass))

typedef struct _ORBitGServant        ORBitGServant;
typedef struct _ORBitGServantClass   ORBitGServantClass;
typedef struct _ORBitGServantPrivate ORBitGServantPrivate;

typedef void (*ORBitGServantSkeleton) (ORBitGServant     *servant,
				       gpointer           retval,
				       gpointer          *args,
				       CORBA_Context      ctx,
				       CORBA_Environment *ev,
				       gpointer           implementation);

typedef enum {
	ORBIT_GMETHOD_DUMMY_FLAG = 1 << 0,
} ORBitGMethodFlags;


struct _ORBitGServant {
	GObject                parent;

	ORBitGServantPrivate  *priv;
};

struct _ORBitGServantClass {
	GObjectClass           parent_class;

	CORBA_boolean    (*is_a)           (ORBitGServant      *servant,
					    const CORBA_char   *repo_id,
					    CORBA_Environment  *ev);

	CORBA_char       *(*get_type_id)    (ORBitGServant     *servant,
					     CORBA_Environment *ev);

	ORBit_IInterface *(*get_iinterface) (ORBitGServant     *servant,
					     const CORBA_char  *repo_id,
					     CORBA_Environment *ev);
};

GType          ORBit_gservant_get_type       (void) G_GNUC_CONST;

void           ORBit_gservant_handle_request (ORBitGServant           *servant,
					      CORBA_Identifier         opname,
					      gpointer                 ret,
					      gpointer                *args,
					      CORBA_Context            ctx,
					      GIOPRecvBuffer          *recv_buffer,
					      CORBA_Environment       *ev);

CORBA_Object      ORBit_gservant_to_reference   (ORBitGServant        *servant);
ORBitGServant    *ORBit_gservant_from_reference (const CORBA_Object    reference);

gboolean          ORBit_gservant_deactivate     (ORBitGServant        *servant);

ORBit_IInterface *ORBit_gservant_get_iinterface (ORBitGServant        *servant,
						 const CORBA_char     *repo_id,
						 CORBA_Environment    *ev);

void              ORBit_gtype_register_static   (GType                 itype,
						 ORBit_IInterface     *iinterface);

void              ORBit_gmethod_register        (const char           *method_name,
						 GType                 itype,
						 guint                 imethod_idx,
						 ORBitGMethodFlags     flags,
						 guint                 class_offset,
						 gpointer              skeleton);

G_END_DECLS

#endif /* __ORBIT_GSERVANT_H__ */
