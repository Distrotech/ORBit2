/*
 * goa-basics.h:
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

#ifndef __GOA_BASICS_H__
#define __GOA_BASICS_H__

#include <glib/gmacros.h>

G_BEGIN_DECLS

#if !defined(_ORBit_GServant_defined)
#define _ORBit_GServant_defined 1
        typedef struct _ORBitGServant *ORBit_GServant;
#else
#error "Include mixup: gobject-adaptor.h included before goa-basics.h"
#endif

#if !defined(ORBIT_DECL_ORBit_GOA) && !defined(_ORBit_GOA_defined)
#define ORBIT_DECL_ORBit_GOA 1
#define _ORBit_GOA_defined 1
	typedef struct _ORBit_GOA *ORBit_GOA;
#endif

typedef struct _ORBitGServant ORBitGServant;

G_END_DECLS

#endif /* __GOA_BASICS_H__ */
