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

#include "IDLType.hh"
#include "IDLTypedef.hh"

void
IDLType::writeInitCode(ostream &ostr, Indent &indent, string const &ident) const {
  // default is to do nothing
}
void
IDLType::writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	ostr << indent << ident << "=" << target << ";" << endl;
}

void
IDLType::writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	ostr << indent << ident << "=" << target << ";" << endl;
}

void
IDLType::writeUnionAccessors(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const{
	string typespec,dcl;
	getCPPStubDeclarator(IDL_PARAM_IN,"",typespec,dcl,activeTypedef);
	ostr
	<< indent << typespec << dcl << " " << id << "() const {" << endl;
	ostr << ++indent << getNSScopedCPPTypeName() << " const &_tmp = reinterpret_cast< "
		 << getNSScopedCPPTypeName() << " const &>(m_target._u." << id << ");" << endl;
	ostr	
	<< indent << "return _tmp;" << endl;
	ostr
	<< --indent << "}" << endl << endl;
}

void
IDLType::writeUnionModifiers(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const{
	string typespec,dcl;
	getCPPStubDeclarator(IDL_PARAM_IN,"param",typespec,dcl,activeTypedef);
	ostr
	<< indent << "void " << id << "(" << typespec << " " << dcl << "){" << endl;
	ostr
	<< ++indent << "_clear_member();" << endl	
	<< indent << "_d(" << discriminatorVal << ");" << endl;	

	ostr << indent << getNSScopedCTypeName() << " const &_tmp = reinterpret_cast< "
		 << getNSScopedCTypeName() << " const &>(param);" << endl;
	writeCDeepCopyCode(ostr,indent,"m_target._u."+id,"_tmp");
	ostr
	<< --indent << "}" << endl << endl;	
}

void
IDLType::writeUnionReferents(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
							IDLTypedef const *activeTypedef = NULL) const{
}

IDLType const &
IDLType::getResolvedType() const {
	IDLType const *type = this;
	IDLTypedef const *td = dynamic_cast<IDLTypedef const*>(this);
	while(td) {
		type = &td->getAlias();
		td = dynamic_cast<IDLTypedef const*>(type);
		if( !td )
			break;
	}
	return *type;
}

string
IDLType::getQualifiedForwarder () const
{
    return getNSScopedCPPTypeName () + "_forwarder";
}
