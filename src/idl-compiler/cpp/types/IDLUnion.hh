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


#ifndef ORBITCPP_TYPES_IDLUNION
#define ORBITCPP_TYPES_IDLUNION

#include "IDLStruct.hh"

// This class derives from IDLStruct because all the parameter passing stuff
// is the same. Yes I know this is a hack.
class IDLUnion : public IDLStruct //public IDLCompound //,public IDLType
{
private:
	IDLType const 	&m_discriminatorType;
public:
	IDLUnion(string const &id,IDL_tree node,
			 IDLType const &discriminatorType, IDLScope *parentscope = NULL);
	IDLType const &getDiscriminatorType() const { return m_discriminatorType;}
	virtual bool isVariableLength() const;

	// returns true if the idlunion has an explicit default clause
	bool hasExplicitDefault() const;
	string getDefaultDiscriminatorValue() const;

	void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
	void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const;
};

#endif //ORBITCPP_TYPES_IDLUNION

