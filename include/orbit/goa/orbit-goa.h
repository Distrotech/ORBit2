/*
 * orbit-goa.h:
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
 */

#ifndef __ORBIT_GOA_H__
#define __ORBIT_GOA_H__

#include <glib/gmacros.h>

#include "gobject-adaptor.h"
#include "orbit-gservant.h"

G_BEGIN_DECLS

/* The ObjectId 8 bytes long and looks like this:
 *
 * .---------- ObjectId -----------,
 * |      4              4         |
 * | object idx | object id random |
 */
#define ORBIT_GOA_OBJECT_ID_LEN (sizeof (CORBA_long) + 4)


ORBit_GOA ORBit_goa_initial_reference  (CORBA_ORB        orb,
					gboolean         only_if_exists);

char     *ORBit_goa_register_servant   (ORBit_GOA         goa,
					ORBitGServant    *servant);

gboolean  ORBit_goa_unregister_servant (ORBit_GOA         goa,
					const char       *object_id);

G_END_DECLS

#endif /* __ORBIT_GOA_H__ */
