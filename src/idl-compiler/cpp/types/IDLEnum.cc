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

#include "IDLEnum.hh"

IDLEnum::IDLEnum(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
: IDLUserDefSimpleType(id,node,parentscope)
{		

	for(IDL_tree curitem = IDL_TYPE_ENUM(node).enumerator_list;
		curitem;
		curitem = IDL_LIST(curitem).next) {
		IDLEnumComponent *enc = new IDLEnumComponent(
			IDL_IDENT(IDL_LIST(curitem).data).str,curitem,parentscope);
		ORBITCPP_MEMCHECK(enc)
		m_elements.push_back(enc);
	}
}

IDLEnum::const_iterator IDLEnum::begin() const
{
	return m_elements.begin();
}

IDLEnum::const_iterator IDLEnum::end() const
{
	return m_elements.end();
}

string IDLEnum::getInvalidReturn() const
{
	return "return "
		+ (*begin())->getNSScopedCTypeName() + ";";
}


string IDLEnum::getDefaultValue(set<string> const &labels)const {
	const_iterator it=begin();
	string result="";

	while(it != end()){
		string test = (*it)->getQualifiedCPPIdentifier();
		if(labels.find(test) == labels.end()){
			result = test;
			break;
		}
		it++;
	}

	return result;
}
