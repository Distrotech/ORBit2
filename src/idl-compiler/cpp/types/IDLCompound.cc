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

#include "IDLCompound.hh"
#include "IDLType.hh"

void
IDLCompound::writeCPackingCode(ostream &header, Indent &indent,ostream &module, Indent &mod_indent) {
	string cname = IDL_IMPL_C_NS_NOTUSED + getQualifiedCIdentifier();
	header
	<< indent << cname << " *_orbitcpp_pack() const {" << endl;
	
	if (size()) {
		header
		<< ++indent << cname << " *_cstruct = " << cname <<"__alloc();" << endl
		<< indent << "if (!_cstruct) throw " IDL_CORBA_NS "::NO_MEMORY();" << endl
		<< indent << "_orbitcpp_pack(*_cstruct);" << endl
		<< indent << "return _cstruct;" << endl;
	}
	else {
		header
		<< ++indent << "return NULL;" << endl;
	}
	header
	<< --indent << '}' << endl;

	header
	<< indent << "void _orbitcpp_pack(" << cname << " &_cstruct) const;" << endl
	<< indent << "void _orbitcpp_unpack(const " << cname << " &_cstruct);" << endl;

	module
	<< mod_indent << "void " << getQualifiedCPPIdentifier(getRootScope())
	<< "::_orbitcpp_pack(" << cname << " &_cstruct) const{" << endl;
	mod_indent++;

	const_iterator first = begin(), last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeCPPStructPacker(module,mod_indent,member.getCPPIdentifier());
	}

	module
	<< mod_indent << '}' << endl << endl;
	mod_indent--;

	module
	<< mod_indent << "void " << getQualifiedCPPIdentifier(getRootScope())
	<< "::_orbitcpp_unpack(const " << cname << " &_cstruct) {" << endl;
	mod_indent++;

	first = begin();
	last = end();

	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeCPPStructUnpacker(module,mod_indent,member.getCPPIdentifier());
	}

	module
	<< mod_indent << '}' << endl << endl;
	mod_indent--;
}

