/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 2000 Andreas Kloeckner
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  Author: Andreas Kloeckner <ak@ixion.net>
 *
 *  Purpose: ORBit-C++ primary include file
 */




#ifndef __ORBITCPP_HH
#define __ORBITCPP_HH



#include "orbitcpp_constants.hh"
#include "orbitcpp_types.hh"
#include "orbitcpp_smartptr.hh"
#include "orbitcpp_tools.hh"
#include "orbitcpp_exception.hh"

#include "orbitcpp_orb.hh"
#include "orbitcpp_poa.hh"

#include "orbitcpp_sequence.hh"
#include "orbitcpp_compound_seq.hh"
#include "orbitcpp_simple_seq.hh"
#include "orbitcpp_string_seq.hh"

#include "orbitcpp_object.hh"
#include "orbitcpp_any.hh"

// hopefully this will be removed when the object_var stuff gets
// sorted out.  - PD
#include "orbitcpp_var_smartptr.hh"

#endif
