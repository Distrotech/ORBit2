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
 *  Author:	Andreas Kloeckner <ak@ixion.net>
 *
 *  Purpose:	IDL compiler type representation
 *
 *
 */

#include "IDLArrayList.hh"

bool
IDLArrayList::doesArrayTypeExist(IDLArray const& array) {
	string typespec, dcl="";
	array.m_elementType.getCPPMemberDeclarator(dcl,typespec,dcl);
	typespec += dcl;
	int length = 1;
	for(const_iterator it = array.begin(); it != array.end(); it++)
		length *= *it;
	IDLArrayKey new_array_key(typespec, length);
	if( m_arraySet.find(new_array_key) == m_arraySet.end() ) {
		m_arraySet.insert(new_array_key);
		return false;
	} else
		return true;
}
