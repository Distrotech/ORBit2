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

IDLArrayList::IDLArrayKey::IDLArrayKey (const string &member_type,
					int           length):
	m_member_type (member_type),
	m_length (length)
{
}

bool
IDLArrayList::IDLArrayKey::operator< (const IDLArrayKey &other_key) const
{
	// Try to discriminate based on length (because that's cheaper
	if (m_length < other_key.m_length)
		return true;

	if (m_length > other_key.m_length)
		return false;

	// If that fails, use type names
	return (m_member_type < other_key.m_member_type);
}


bool
IDLArrayList::array_exists (const IDLArray &array)
{
	string member_type = array.m_element_type.get_cpp_member_typename ();
	
	int length = 1;
	for (IDLArray::const_iterator i = array.begin(); i != array.end(); i++)
		length *= *i;
	
	IDLArrayKey new_array_key (member_type, length);

	if (m_arrays.find (new_array_key) == m_arrays.end ())
	{
	    m_arrays.insert (new_array_key);
	    return false;
	} else {
	    return true;
	}
}
