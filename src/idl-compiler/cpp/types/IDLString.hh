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


#ifndef ORBITCPP_TYPES_IDLSTRING
#define ORBITCPP_TYPES_IDLSTRING

#include "IDLType.hh"

class IDLString : public IDLType
{
public:

	string getNSScopedCTypeName() const {
		return getCTypeName();
	}

	string getNSScopedCPPTypeName() const {
		return "char *";
	}
	
	bool isVariableLength() const {
		return true;
	}
	
	// misc stuff
	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const {
		typespec = IDL_CORBA_NS "::String_mgr";
		dcl = id;
	}
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const {
		ostr
		<< indent << "typedef char *" << ' '
		<< dest.getCPPIdentifier() << ';' << endl;
	}
	void getCPPConstantDeclarator(string const &id,string &typespec,string &dcl) {
		typespec = "char ";
		dcl = "*const "+id;
	}

	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = "char const";
		dcl = "*_par_" + id;
	}
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << id << " = _par_" << id << ';' << endl;
	}
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "_cstruct." << id
		<< " = " IDL_CORBA_NS "::string_dup("<< id << ");" << endl;
	}
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << id
		<< " = " IDL_CORBA_NS "::string_dup(_cstruct." << id << ");" << endl;
	}

	
	void
	writeUnionModifiers(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
					  IDLTypedef const *activeTypedef = NULL) const{
		ostr
		<< indent << "void " << id << "(char const *param){" << endl;
		ostr
		<< ++indent << "_clear_member();" << endl		
		<< indent << "_d(" << discriminatorVal << ");" << endl;	
		ostr	
		<< indent << "m_target._u." << id << " = " << "CORBA::string_dup(param);" << endl;
		ostr
		<< --indent << "}" << endl << endl;	

		ostr
		<< indent << "void " << id << "(CORBA::String_var param){" << endl;
		ostr
		<< ++indent << "_clear_member();" << endl;
		ostr	
		<< ++indent << "_d(" << discriminatorVal << ");" << endl;	
		ostr	
		<< indent << "m_target._u." << id << " = " << "CORBA::string_dup(param);" << endl;
		ostr
		<< --indent << "}" << endl << endl;	

		ostr
		<< indent << "void " << id << "(char *param){" << endl;
		ostr
		<< ++indent << "_clear_member();" << endl;
		ostr	
		<< ++indent << "_d(" << discriminatorVal << ");" << endl;	
		ostr	
		<< indent << "m_target._u." << id << " = param;" << endl;
		ostr
		<< --indent << "}" << endl << endl;	
	}

	void
	writeUnionAccessors(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
					  IDLTypedef const *activeTypedef = NULL) const{
		ostr
		<< indent << "char const *" << id << "() const {" << endl;
		ostr
		<< ++indent << "return m_target._u."<< id <<";" << endl;
		ostr
		<< --indent << "}" << endl << endl;
	}
	

	
	// sequence stuff

	string getCTypeName() const {
		return "CORBA_string";
	}
	
	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef=NULL) const {
		switch (attr) {
		case IDL_PARAM_IN:
			typespec = "char const";
			dcl = "*" + id;
			break;
		case IDL_PARAM_INOUT:
			typespec = "char";
			dcl = "*&" + id;
			break;
		case IDL_PARAM_OUT:
			typespec = IDL_CORBA_NS "::String_out";
			dcl = id;
			break;
		}
	}
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		switch (attr) {
		case IDL_PARAM_INOUT:
			return "&"+id;
		case IDL_PARAM_OUT:
			return "&(char *&) "+id;
		default:
			return id;
		}
	}

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = "char";
		dcl = "*" + id;
	}
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "char *_retval = NULL;" << endl;
	}
	string getCPPStubReturnAssignment() const {
		return "_retval = ";
	}
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "return _retval;" << endl;
	}

	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const {
		typespec = attr == IDL_PARAM_IN ? "char const" : "char";
		dcl = attr == IDL_PARAM_IN ? "*"+id : "**"+id;
	}
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		return attr == IDL_PARAM_IN ? id : "*"+id;
	}

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
		typespec = "char";
		dcl = "*"+id;
	}
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "char *_retval = NULL;" << endl;
	}
	string getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
		return "_retval = ";
	}
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "return _retval;" << endl;
	}
	string getInvalidReturn() const {
		return "return NULL;";
	}

	void writeInitCode(ostream &ostr, Indent &indent, string const &ident) const {
		ostr << indent << ident << " = CORBA::string_dup(\"\");" << endl;
	}
	
	void writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
		ostr << indent << ident << " = CORBA::string_dup(" << target << ");" << endl;
	}
	void writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
		ostr << indent << ident << " = CORBA_string_dup(" << target << ");" << endl;
	}
};

#endif //ORBITCPP_TYPES_IDLSTRING
