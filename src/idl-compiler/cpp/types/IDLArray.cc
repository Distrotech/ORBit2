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

#include "IDLArray.hh"
#include "error.hh"
#include "pass.hh"
#include "pass_xlate.hh"
#include <strstream>

void
IDLArray::writeInitCode(ostream &ostr, Indent &indent, string const &ident) const {
	if(m_elementType.isVariableLength()){
		// write the initialisation code
		char *dimItVar = new char[2];
		dimItVar[0] = 'a';
		dimItVar[1] = '\0';

		string dimsStr;
		for(const_iterator it = begin(); it != end(); it++){
			ostr
			<< indent++ << "for (CORBA::ULong " << dimItVar << "=0;"<<dimItVar<<"<"<< *it << ";"<<dimItVar<<"++){" << endl;
			dimsStr += string("[")+dimItVar+"]";
			dimItVar[0]++;
		}
		delete[] dimItVar;
	
		m_elementType.writeInitCode(ostr,indent,ident+dimsStr);
	
		for(const_iterator it = begin(); it != end(); it++){
			ostr
			<< --indent << "}" << endl;
		}
	}
}

void
IDLArray::writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	char *dimItVar = new char[2];
	dimItVar[0] = 'a';
	dimItVar[1] = '\0';
	
	string dimsStr;
	for(const_iterator it = begin(); it != end(); it++){
		ostr
		<< indent++ << "for (CORBA::ULong " << dimItVar << "=0;"<<dimItVar<<"<"<< *it << ";"<<dimItVar<<"++){" << endl;
		dimsStr += string("[")+dimItVar+"]";
		dimItVar[0]++;
	}
	delete[] dimItVar;
	
	m_elementType.writeCPPDeepCopyCode(ostr,indent,ident+dimsStr,target+dimsStr);
	
	for(const_iterator it = begin(); it != end(); it++){
		ostr
			<< --indent << "}" << endl;
	}
}

void
IDLArray::writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	char *dimItVar = new char[2];
	dimItVar[0] = 'a';
	dimItVar[1] = '\0';
	
	string dimsStr;
	for(const_iterator it = begin(); it != end(); it++){
		ostr
		<< indent++ << "for (CORBA_unsigned_long " << dimItVar << "=0;"<<dimItVar<<"<"<< *it << ";"<<dimItVar<<"++){" << endl;
		dimsStr += string("[")+dimItVar+"]";
		dimItVar[0]++;
	}
	delete[] dimItVar;
	
	m_elementType.writeCDeepCopyCode(ostr,indent,ident+dimsStr,target+dimsStr);
	
	for(const_iterator it = begin(); it != end(); it++){
		ostr
			<< --indent << "}" << endl;
	}
}


void
IDLArray::getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const {
	if( activeTypedef ) {
		typespec = activeTypedef->getQualifiedCPPIdentifier(activeTypedef->getRootScope());
		dcl = id;
	}
	else {
		m_elementType.getCPPMemberDeclarator(id, typespec, dcl, activeTypedef);
		strstream dcl_str;
		for(const_iterator it = begin(); it != end(); it++){
			 dcl_str << "[" << *it << "]";
		}
		dcl += string(dcl_str.str(), dcl_str.pcount());
	}
}

void
IDLArray::writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					   IDLElement &dest,IDLScope const &scope,
					   IDLTypedef const *activeTypedef = NULL) const {

	string typespec,dcl,str_static="";
	m_elementType.getCPPMemberDeclarator("",typespec,dcl);
	ostr
	<< indent << "typedef " << typespec + dcl << ' '
	<< dest.getCPPIdentifier();

	int i=0;
	
	for(const_iterator it = begin(); it != end(); it++){
	  i++;
		ostr << "[" << *it << "]";
	}
	ostr << ";" << endl;
	
	ostr
	<< indent << "typedef " << typespec + dcl << ' '
	<< dest.getCPPIdentifier() << "_slice";

	const_iterator it = begin();
	for(++it; it != end(); ++it){  // an array slice is the all the dims except
	  ostr << "[" << *it << "]"; //   the first one
	}
	ostr << ";" << endl;

	if( scope.getTopLevelInterface() )
		str_static = "static ";
	ostr
	<< indent << str_static << "inline "
	<< dest.getCPPIdentifier() << "_slice *"
	<< dest.getCPPIdentifier() << "_alloc() {\n";

	
	string allocfunc = idlGetCast(IDL_IMPL_C_NS_NOTUSED  +
								  dest.getQualifiedCIdentifier()+"__alloc()",
								  dest.getCPPIdentifier() +"_slice *");
	ostr
	<< ++indent << dest.getCPPIdentifier() << "_slice *array =" << allocfunc << ";\n";
	writeInitCode(ostr,indent,"array");
	// and return the array
	ostr
	  << indent << "return array;\n";
	
	ostr
	<< --indent  << "}\n\n";

	ostr
	<< indent++ << str_static << "inline void "
	<< dest.getCPPIdentifier() << "_copy("<< dest.getCPPIdentifier() << "_slice *dest, "
	<< dest.getCPPIdentifier() << "_slice const *source) {\n";
	writeCPPDeepCopyCode(ostr,indent,"dest","source");		
	ostr
	<< --indent  << "}\n\n";

	ostr
	<< indent << str_static << "inline "
	<< dest.getCPPIdentifier() << "_slice *"
	<< dest.getCPPIdentifier() << "_dup("<< dest.getCPPIdentifier() << "_slice const * target) {\n";
	ostr
	<< ++indent << dest.getCPPIdentifier() << "_slice *array = "
	<< dest.getCPPIdentifier() << "_alloc();" << endl;
	ostr
	<< indent << dest.getCPPIdentifier() << "_copy(array,target);" << endl;
	
	ostr
	  << indent << "return array;\n";
	
	ostr
	<< --indent  << "}\n\n";

	
	ostr
	<< indent << str_static << "inline "
	<< "void "
	<< dest.getCPPIdentifier() << "_free(" << dest.getCPPIdentifier() << "_slice *a) {\n";
	ostr
	<< ++indent << "CORBA_free(a);\n";
	ostr
	<< --indent  << "}\n\n";

	int length = 1;
	for(it = begin(); it != end(); it++)
		length *= *it;

	if (!isVariableLength()){
		ostr
		<< indent << "typedef "IDL_IMPL_NS "::ArrayFixed_var< "
		<< dest.getCPPIdentifier() << "_slice," << length << " > "
		<< dest.getCPPIdentifier() << "_var;" << endl << endl;

		ostr
		<< indent << "typedef " << dest.getCPPIdentifier() << " "
		<< dest.getCPPIdentifier() << "_out;" << endl << endl;
		
		ostr
		<< indent << "typedef "IDL_IMPL_NS "::ArrayFixed_forany< "
		<< dest.getCPPIdentifier() << "_slice," << length << " > "
		<< dest.getCPPIdentifier() << "_forany;" << endl << endl;
	} else {
		ostr
		<< indent << "typedef "IDL_IMPL_NS "::ArrayVariable_var< "
		<< dest.getCPPIdentifier() << "_slice," << length << " > "
		<< dest.getCPPIdentifier() << "_var;" << endl << endl;
		ostr
		<< indent << "typedef "IDL_IMPL_NS "::ArrayVariable_out< "
		<< dest.getCPPIdentifier() << "_slice," << length << " > "
		<< dest.getCPPIdentifier() << "_out;" << endl << endl;
		
		ostr
		<< indent << "typedef "IDL_IMPL_NS "::ArrayVariable_forany< "
		<< dest.getCPPIdentifier() << "_slice," << length << " > "
		<< dest.getCPPIdentifier() << "_forany;" << endl << endl;
	}	

	ostr
	<< indent << "typedef " IDL_IMPL_NS "::ArrayProperties< "
	<< dest.getQualifiedCPPIdentifier() << "_slice," << length << " > "
	<< dest.getCPPIdentifier() << "Props;" << endl << endl;
	
	if( state.m_array_list.doesArrayTypeExist(*this) == false ) {
		ORBITCPP_MEMCHECK( new IDLWriteArrayProps(*this,dest,state,*state.m_pass_xlate) );
		ORBITCPP_MEMCHECK( new IDLWriteArrayAnyFuncs(*this,dest,state,*state.m_pass_xlate) );
	}
}

void
IDLArray::getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
										IDLTypedef const *activeTypedef = NULL) const {
	getCPPMemberDeclarator(id, typespec, dcl, activeTypedef);
	typespec = "const " + typespec;
	dcl = "_par_" + dcl;
}

void
IDLArray::writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
	if(activeTypedef) {
		ostr
		<< indent << activeTypedef->getQualifiedCPPIdentifier() << "_copy("
		<< id << ", _par_" << id << ");" << endl;
	}
	else
		writeCPPDeepCopyCode(ostr,indent, id, "_par_"+id);
}

void
IDLArray::writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
								  IDLTypedef const *activeTypedef = NULL) const {
	if( activeTypedef ) {
		ostr
		<< indent << activeTypedef->getQualifiedCPPIdentifier() << "_copy("
		<< idlGetCast("_cstruct."+id,activeTypedef->getQualifiedCPPIdentifier() + "_slice*")
		<< ", " << id << ");" << endl;
	}
	else {
		string typespec, dcl;
		m_elementType.getCPPMemberDeclarator("",typespec,dcl);
		typespec += dcl;
		writeCPPDeepCopyCode(ostr, indent, "("+idlGetCast("_cstruct."+id, typespec+"*")+")", id );
	}
}

void
IDLArray::writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
									IDLTypedef const *activeTypedef = NULL) const {
	if( activeTypedef ) {
		ostr
		<< indent << activeTypedef->getQualifiedCPPIdentifier() << "_copy("
		<< id << ", "
		<< idlGetCast("_cstruct."+id,activeTypedef->getQualifiedCPPIdentifier() + "_slice*")
		<< ");" << endl;
	}
	else {
		string typespec, dcl;
		m_elementType.getCPPMemberDeclarator("",typespec,dcl);
		typespec += dcl;
		writeCPPDeepCopyCode(ostr, indent, id,
				"("+idlGetCast("_cstruct."+id, typespec+"*")+")" );
	}
}


void
IDLArray::writeUnionModifiers(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const{
	string typespec,dcl;
	getCPPStubDeclarator(IDL_PARAM_IN,"param",typespec,dcl,activeTypedef);
	ostr
	<< indent << "void " << id << "(" << typespec << " " << dcl << "){" << endl;
	ostr
	<< ++indent << "_clear_member();" << endl	
	<< indent << "_d(" << discriminatorVal << ");" << endl;

	ostr << indent << activeTypedef->getNSScopedCTypeName() << " const &_tmp = reinterpret_cast< "
		 << activeTypedef->getNSScopedCTypeName() << " const &>(param);" << endl;

	writeCDeepCopyCode(ostr,indent,"m_target._u."+id,"_tmp");

	ostr
	<< --indent << "}" << endl << endl;	
}

void
IDLArray::writeUnionAccessors(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
									IDLTypedef const *activeTypedef = NULL) const{
	string typespec,dcl;
	g_assert(activeTypedef);
	getCPPStubDeclarator(IDL_PARAM_IN,"",typespec,dcl,activeTypedef);
	ostr
	<< indent << typespec << dcl << "_slice *" << id << "() const {" << endl;
	ostr << indent << activeTypedef->getNSScopedCPPTypeName() << "_slice const *_tmp = reinterpret_cast< "
		 << activeTypedef->getNSScopedCPPTypeName() << "_slice const *>(m_target._u." << id << ");" << endl;
	ostr	
	<< ++indent << "return _tmp;" << endl;
	ostr
	<< --indent << "}" << endl << endl;
}

void
IDLArray::writeUnionReferents(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
							IDLTypedef const *activeTypedef = NULL) const{
	string typespec,dcl;
	g_assert(activeTypedef);
	getCPPStubDeclarator(IDL_PARAM_IN,"",typespec,dcl,activeTypedef);
	ostr
	<< indent << activeTypedef->getNSScopedCPPTypeName()<< "_slice *" << id << "() {" << endl;
	ostr
	<< ++indent << activeTypedef->getNSScopedCPPTypeName() << "_slice *_tmp = reinterpret_cast< "
	<< activeTypedef->getNSScopedCPPTypeName() << "_slice *>(m_target._u." << id << ");" << endl;
	ostr	
	<< indent << "return _tmp;" << endl;
	ostr
	<< --indent << "}" << endl << endl;
}

// stub stuff
void
IDLArray::getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef=NULL) const {
	typespec = attr == IDL_PARAM_IN ? "const " : "";
	typespec += activeTypedef ?
		activeTypedef->getQualifiedCPPIdentifier() : getTypeName();
	if (attr == IDL_PARAM_OUT) typespec += "_out";
	dcl = id;
}


void
IDLArray::getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const {
	g_assert(activeTypedef);
	typespec = attr == IDL_PARAM_IN ? "const " : "";
	typespec += activeTypedef->getNSScopedCTypeName();
	dcl = id;
	if( attr == IDL_PARAM_OUT && isVariableLength()) {
		typespec += "_slice";
		dcl = "**" + dcl;
	}
}

void
IDLArray::getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									 IDLTypedef const *activeTypedef = NULL) const {
	g_assert(activeTypedef);
	typespec = activeTypedef->getQualifiedCPPIdentifier() + "_slice";
	dcl = "*" + id;
}

void
IDLArray::getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
	g_assert(activeTypedef);
	typespec = activeTypedef->getNSScopedCTypeName() + "_slice";
	dcl = "*" + id;
}


string
IDLArray::getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
	string typespec,dcl;
	getCSkelDeclarator(attr,"",typespec,dcl,activeTypedef);
	
	string term;
	if(!isVariableLength()){
	  term = idlGetCast(id,typespec+dcl+"_slice *&");
	} else {
	  term = idlGetCast((attr == IDL_PARAM_OUT ? "&"+id+".ptr()" : id),
	  					(attr == IDL_PARAM_OUT ? typespec+dcl : typespec+dcl+"_slice *&"));
	}
	return term;
}

string
IDLArray::getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
	string typespec,dcl;
	getCPPStubDeclarator(attr,"",typespec,dcl,activeTypedef);
	string term;

	// we can't use the foo_out type for marshalling - instead use a
	// foo_slice *&
	if(attr == IDL_PARAM_OUT)
	  typespec = activeTypedef ?
		activeTypedef->getQualifiedCPPIdentifier() : getTypeName();
	
	if(!isVariableLength()){
	  term = idlGetCast(id,typespec + "_slice *&");
	} else {
	  term = idlGetCast((attr == IDL_PARAM_OUT ? "*" : "") + id,
						typespec + "_slice *&");
	}
	
	return term;
}

