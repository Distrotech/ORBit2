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


#ifndef ORBITCPP_TYPES_IDLARRAYLIST
#define ORBITCPP_TYPES_IDLARRAYLIST

#include <language.hh>
#include "IDLArray.hh"

class IDLArrayList
{
	struct IDLArrayKey {
		string m_type;
		int m_length;
		
		IDLArrayKey(string const& type, int length)
			: m_type(type), m_length(length) {}
		bool operator <(IDLArrayKey const & key) const {
			if( m_length < key.m_length )
				return true;
			else if( m_length == key.m_length )
				return (m_type < key.m_type);
			else	
				return false;
		}
	};
	typedef std::vector<int>::const_iterator const_iterator;
	std::multiset<IDLArrayKey> m_arraySet;
public:
	IDLArrayList() {}
	bool doesArrayTypeExist(IDLArray const& array);
	void clear() { m_arraySet.clear(); }
};

#endif //ORBITCPP_TYPES_IDLARRAYLIST
