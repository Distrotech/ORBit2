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


#ifndef ORBITCPP_TYPES_IDLEXCEPTION
#define ORBITCPP_TYPES_IDLEXCEPTION

#include "IDLCompound.hh"

class IDLException : public IDLCompound
{
public:
	IDLException(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLCompound(id,node,parentscope) {
	}
	bool isType() {
		// this is required for type container generation
		return true;
	}
	string getRepositoryId() {
		return IDL_IDENT_REPO_ID(IDL_EXCEPT_DCL(getNode()).ident);
	};

	void stub_check_and_propagate (ostream &ostr,
				       Indent  &indent) const;
};

#endif //ORBITCPP_TYPES_IDLEXCEPTION

