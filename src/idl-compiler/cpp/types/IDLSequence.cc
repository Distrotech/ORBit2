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

#include "IDLSequence.hh"
#include "IDLTypedef.hh"
#include "pass.hh"
#include "pass_xlate.hh"
#include <strstream>

bool IDLSequenceComp::operator()(IDLSequence const *s1, IDLSequence const *s2) const
{
  if( s1->m_length < s2->m_length )
  	return true;
  else if( s1->m_length == s2->m_length)
	{
  	string ts1, ts2, dcl1 = "", dcl2 = "";
  	s1->m_elementType.getCPPMemberDeclarator(dcl1, ts1, dcl1);
  	s2->m_elementType.getCPPMemberDeclarator(dcl2, ts2, dcl2);
  	return( (ts1 + dcl1) < (ts2 + dcl2) );
  }
  else
  	return false;
}

void
IDLSequence::writeInitCode(ostream &ostr, Indent &indent, string const &ident) const {
	
	ostr
	<< indent++ << "for (CORBA::ULong i=0;i<" << ident << ".length();i++){" << endl;
	getElementType().writeInitCode(ostr,indent,ident+"[i]");	
	ostr
	<< --indent << "}" << endl;
}


void
IDLSequence::writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	ostr << indent << ident << ".length(" << target << ".length());" << endl;
	ostr
	<< indent++ << "for (CORBA::ULong i=0;i<" << ident << ".length();i++){" << endl;
	getElementType().writeCPPDeepCopyCode(ostr,indent,ident+"[i]",target+"[i]");	
	ostr
	<< --indent << "}" << endl;
}

void
IDLSequence::writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	ostr << indent << ident << "._length = " << target << "._length;" << endl;
	ostr << indent << ident << "._maximum = " << target << "._maximum;" << endl;
	
	ostr << indent << "if(" << ident << "._release == 1) { CORBA_free(" << ident << "._buffer); }" << endl;
	ostr << indent << ident+"._buffer = "
	     << "static_cast<" << m_elementType.getCTypeName () << "*> ("
	     << getNSScopedCTypeName() << "_allocbuf("<< ident <<"._length));" << endl;
	
	ostr << indent << ident << "._release = 1;" << endl;
	ostr
	<< indent++ << "for (CORBA_unsigned_long i=0;i<" << ident << "._length;i++){" << endl;
	getElementType().writeCDeepCopyCode(ostr,indent,ident+"._buffer[i]",target+"._buffer[i]");	
	ostr
	<< --indent << "}" << endl;
}

bool
IDLSequence::isVariableLength() const {
	return true;
}


void
IDLSequence::getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
	if(activeTypedef)
		typespec = activeTypedef->getQualifiedCPPIdentifier(activeTypedef->getRootScope());
	else
		typespec = getCPPType();
	dcl = id;
}



string
IDLSequence::getCPPType() const {
	std::strstream id;
	string typespec, dcl;
	
	m_elementType.getCPPMemberDeclarator("",typespec,dcl);

	if (m_length == 0) {
		id << IDL_IMPL_NS_ID "::UnboundedSequenceTmpl< ";
		id << typespec << dcl << ", "
			<< getNSScopedCTypeName() << ">";
	} else {
		id << IDL_IMPL_NS_ID "::BoundedSequenceTmpl< ";
		id << typespec << dcl << "," << getNSScopedCTypeName() << ","
		<< m_length << ">";
	}
	
	return string(id.str(), id.pcount());
}

string
IDLSequence::getNSScopedCTypeName() const {
	// some types are defined in the orbit library (and so aren't in
	// the _orbitcpp::c namespace)
	if(getCTypeName() == "CORBA_sequence_CORBA_any"){
		return getCTypeName();
	} else {
		return IDL_IMPL_C_NS_NOTUSED + getCTypeName();
	}
}


void
IDLSequence::writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
						  IDLElement &dest,IDLScope const &scope,
						  IDLTypedef const *activeTypedef = NULL) const {
	if (activeTypedef == NULL) { // if this isn't a typedef of a typedef...
		string id = getCPPType();
		ostr
		<< indent << "typedef " << id
		<< " " << dest.getCPPIdentifier() << ";" << endl << endl;
				
		ostr
		<< indent << "typedef " << IDL_IMPL_NS << "::Sequence_var< " << dest.getCPPIdentifier() << "> "
		<< dest.getCPPIdentifier() << "_var;" << endl;

		ostr
		<< indent << "typedef " << IDL_IMPL_NS << "::Sequence_out< " << dest.getCPPIdentifier() << "> "
		<< dest.getCPPIdentifier() << "_out;" << endl << endl;

	} else {
		ostr
		<< indent << "typedef " << activeTypedef->getQualifiedCPPIdentifier() << " "
		<< dest.getCPPIdentifier() << ";" << endl
		<< indent << "typedef " << activeTypedef->getQualifiedCPPIdentifier() << "_var "
		<< dest.getCPPIdentifier() << "_var;" << endl
		<< indent << "typedef " << activeTypedef->getQualifiedCPPIdentifier() << "_out "
		<< dest.getCPPIdentifier() << "_out;" << endl << endl;
	}
}

void
IDLSequence::writeCPPSpecCode(ostream &ostr, Indent &indent, IDLCompilerState &state) const {
	string id = getCPPType();
	
	if( state.m_seq_list.doesSeqTypeExist(*this) == true )
		return;
	
  g_warning("ORBit -lc++: Sequences not fully implemented yet.\n");
  //The following results in multiple identical template specializations being generated, one for each use of the sequence type:	
//	ostr
//	<< indent << "inline void *" << id
//	<< "::operator new(size_t) {" << endl;
//
//	ostr
//	<< ++indent << "return "
//	<< getNSScopedCTypeName() << "__alloc();" << endl;
//	ostr
//	<< --indent << "}" << endl << endl;

	string typespec, dcl;
	m_elementType.getCPPMemberDeclarator(id, typespec, dcl);

//	if(m_elementType.isVariableLength()) {		
//		ostr
//		<< indent << "inline " << typespec << " *" << dcl
//		<< "::allocbuf(CORBA::ULong len) {" << endl;
//		// create the sequence
//		ostr
//		<< ++indent << typespec << " *buf = reinterpret_cast< "+ typespec +"*>("
//		<< getNSScopedCTypeName()
//		<< "_allocbuf(len));" << endl
//		<< indent++ << "for (CORBA::ULong h=0;h<len;h++){" << endl;
//		m_elementType.writeInitCode(ostr,indent,"buf[h]");	
//		ostr
//		<< --indent << "}" << endl;
//
//		// and return it
//		ostr
//		<< indent << "return buf;" << endl;
//		ostr
//		<< --indent << "};" << endl << endl;
//	} else {
//		ostr
//		<< indent << "inline " << typespec << " *" << dcl
//		<< "::allocbuf(CORBA::ULong len) {" << endl;
//		ostr
//		<< ++indent << "return reinterpret_cast< "+ typespec +"*>("
//		<< getNSScopedCTypeName() << "_allocbuf(len));" << endl;
//		ostr
//		<< --indent << "};" << endl << endl;
//	}

	IDLWriteAnyFuncs::writeInsertFunc(ostr, indent, IDLWriteAnyFuncs::FUNC_COPY,
			id, getCTypeName());
	IDLWriteAnyFuncs::writeInsertFunc(ostr, indent, IDLWriteAnyFuncs::FUNC_NOCOPY,
			id, getCTypeName());
	IDLWriteAnyFuncs::writeExtractFunc(ostr, indent, IDLWriteAnyFuncs::FUNC_NOCOPY,
			id, getCTypeName());
}

void
IDLSequence::getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
										IDLTypedef const *activeTypedef = NULL) const {
	getCPPMemberDeclarator(id, typespec, dcl, activeTypedef);
	typespec = "const " + typespec;
	dcl = "&_par_" + dcl;
}

void
IDLSequence::writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << id << " = _par_" << id << ';' << endl;
}

void
IDLSequence::writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
								  IDLTypedef const *activeTypedef = NULL) const {
	string type;
	if( activeTypedef )
		type = activeTypedef->getQualifiedCPPIdentifier();
	else
		type = getCPPType();
	ostr
	<< indent << idlGetCast("_cstruct."+id, type+"&") << " = " << id
	<< ';' << endl;
	//ostr << indent << id << "._orbitcpp_set_internal_release_flag(false);" << endl;
}

void
IDLSequence::writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
									IDLTypedef const *activeTypedef = NULL) const {
	string type;
	if( activeTypedef )
		type = activeTypedef->getQualifiedCPPIdentifier();
	else
		type = getCPPType();
	//	ostr
	//<< indent << "_cstruct."+id+"._maximum = _cstruct."+id+"._length; "
	//<<"// hack to get round the 'maximum not marshalled' bug in ORBit" << endl;
	ostr
	<< indent << id << " = "
	<< idlGetCast("_cstruct." + id,"const" + type+"&")
	<< ';' << endl;
}


void
IDLSequence::writeUnionReferents(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
							IDLTypedef const *activeTypedef = NULL) const{

	g_assert(activeTypedef);      //activeTypedef cannot be null for sequences
	ostr
	<< indent << activeTypedef->getNSScopedCPPTypeName() << " &" << id << "() {" << endl;
	ostr	
	<< ++indent << "return reinterpret_cast< " << activeTypedef->getNSScopedCPPTypeName()
	<< "&>(m_target._u." << id << ");" << endl;
	ostr
	<< --indent << "}" << endl;
}


void IDLSequence::getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
									   IDLTypedef const* activeTypedef=NULL) const {
	dcl = id;

	g_assert(activeTypedef);      //activeTypedef cannot be null for sequences
	
	switch (attr) {
	case IDL_PARAM_IN:
		typespec = "const " + activeTypedef->getQualifiedCPPIdentifier();
		dcl = '&' + dcl;
		break;
	case IDL_PARAM_INOUT:
		typespec = activeTypedef->getQualifiedCPPIdentifier();
		dcl = '&' + dcl;
		break;
	case IDL_PARAM_OUT:
		typespec = activeTypedef->getQualifiedCPPIdentifier() + "_out";
		break;
	}
}

string
IDLSequence::getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
									 IDLTypedef const *activeTypedef = NULL) const {

	string typespec,dcl, retval;
	getCSkelDeclarator(attr,"",typespec,dcl,activeTypedef);

	switch (attr) {
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		retval = idlGetCast("&"+id,typespec+dcl);
		break;
	case IDL_PARAM_OUT:
		retval = idlGetCast("&"+id+".ptr()",typespec+dcl);
		break;
	}

	
	return retval;
}

void
IDLSequence::getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
IDLTypedef const *activeTypedef = NULL) const {
	typespec = activeTypedef->getQualifiedCPPIdentifier();
	if (isVariableLength())
		dcl = "*" + id;
	else
		dcl = id;
}

void
IDLSequence::writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
IDLTypedef const *activeTypedef = NULL) const {

	ostr
	<< indent << activeTypedef->getNSScopedCTypeName();
	if (isVariableLength())
		ostr << " *_retval = NULL;" << endl;
	else
		ostr << " _retval;" << endl;
}


string
IDLSequence::getCPPStubReturnAssignment() const {
	return "_retval = ";		
}

void
IDLSequence::writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
											 IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << "return reinterpret_cast< " << activeTypedef->getQualifiedCPPIdentifier();
	if(isVariableLength())
		ostr << "*";
	else
		ostr << "&";
	ostr << ">(_retval);" << endl;
}		


void
IDLSequence::getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const {
	typespec = activeTypedef->getNSScopedCTypeName();

	switch (attr) {
	case IDL_PARAM_IN:
		typespec = "const " + typespec;
		dcl = '*' + id;
		break;
	case IDL_PARAM_INOUT:
		dcl = '*' + id;
		break;
	case IDL_PARAM_OUT:
		dcl = "**" + id;
	}
	
}




void
IDLSequence::writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
									   IDLTypedef const *activeTypedef = NULL) const {
	// no demarshalling code required
}

string
IDLSequence::getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
									 IDLTypedef const *activeTypedef = NULL) const {
	string typespec,dcl;
	getCPPStubDeclarator(attr,"",typespec,dcl,activeTypedef);

	switch (attr) {
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		return idlGetCast("*"+id,typespec+dcl);
	case IDL_PARAM_OUT:
		if(isVariableLength())
			return idlGetCast("*"+id,activeTypedef->getQualifiedCPPIdentifier()+"*&");
		else
			return idlGetCast("*"+id,typespec+dcl);
	}
	return "";
}



void
IDLSequence::writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
									 IDLTypedef const *activeTypedef = NULL) const {
	// no marshalling code required
}


void
IDLSequence::getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
									  IDLTypedef const *activeTypedef = NULL) const {
	typespec = activeTypedef->getNSScopedCTypeName();
	dcl = "*" + id;
}

void
IDLSequence::writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
										IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << activeTypedef->getQualifiedCPPIdentifier();
	if(isVariableLength())
		ostr << " *_retval = NULL;" << endl;
	else
		ostr << " _retval;" << endl;
}

string
IDLSequence::getCPPSkelReturnAssignment(bool passthru,
										IDLTypedef const *activeTypedef = NULL) const {
	return "_retval = ";		
}

void
IDLSequence::writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
										   IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << "return reinterpret_cast< "
	<< activeTypedef->getNSScopedCTypeName()
	<< "*>(_retval);" << endl;
}


string
IDLSequence::getInvalidReturn() const {
	return "return NULL;";
}
