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
 *  Remarks:
 *    It is convention that the C struct is available by reference as "_cstruct"
 *    in the writeCPPStructPacker/Unpacker() contexts.
 *
 */




#ifndef ORBITCPP_TYPES
#define ORBITCPP_TYPES




#include "error.hh"
#include "language.hh"
#include "types.hh"
#include "types/IDLType.hh"
#include "types/IDLAny.hh"
#include "types/IDLArray.hh"
#include "types/IDLVoid.hh"
#include "types/IDLString.hh"
#include "types/IDLBoolean.hh"
//#include "types/IDLSequence.hh"
#include "types/IDLObject.hh"
#include "types/IDLTypeCode.hh"
//#include "types/IDLStruct.hh"
#include "types/IDLEnum.hh"
//#include "types/IDLUnion.hh"
#include "types/IDLException.hh"

class IDLTypedef;

class IDLTypeParser
{
public:
	IDLTypeParser(IDLCompilerState& state)
		: m_state(state) {}
	~IDLTypeParser();
	IDLType *parseTypeSpec(IDLScope &scope, IDL_tree typespec);
	IDLType *parseDcl(IDL_tree dcl,IDLType *typespec, string &id);

protected:
	IDLCompilerState& m_state;
	vector<IDLType *> m_anonymous_types;
};




#endif //ORBITCPP_TYPES
