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
 *          Phil Dawes <philipd@users.sourceforge.net>
 *			John K. Luebs <jkluebs@marikarpress.com>
 *
 *  Purpose: IDL compiler type representation
 *
 */




#include "types.hh"
#include "pass_xlate.hh"
#include <strstream>


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


// readymade types ------------------------------------------------------------
static class IDLVoid : public IDLType {
public:

	string getNSScopedCTypeName() const {
		return getCTypeName();
	}

	string getNSScopedCPPTypeName() const {
		throw IDLExVoid();
		return "";
	}
	
	virtual bool isVariableLength() const {
		return false;  // default case for most types
	}

	// misc stuff
	void getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}

	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec, string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	
	// sequence stuff
	string getCTypeName() const {
		throw IDLExVoid();
	}

	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,
							  string &typespec,string &dcl,
							  IDLTypedef const* activeTypedef=NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStubMarshalCode(IDL_param_attr attr,string const &id,
								 ostream &ostr, Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPStubDemarshalCode(IDL_param_attr attr,string const &id,
								   ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec, string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = "void";
		dcl = id;
	}
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const {
	}
	string getCPPStubReturnAssignment() const {
		return "";
	}
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
	}

	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}
	void writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const {
		throw IDLExVoid();
	}

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
		typespec = "void";
		dcl = id;
	}
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									IDLTypedef const *activeTypedef = NULL) const {
	}
	string getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
		return "";
	}
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const {
	}
	string getInvalidReturn() const {
		return "return;";
	}
} idlVoid;



class IDLString : public IDLType {
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
} idlString;




#define ORBITCPP_MAKE_SIMPLE_TYPE(name,cname) \
	static class IDL##name : public IDLSimpleType { \
		string getTypeName() const { \
			return IDL_CORBA_NS "::" #name; \
		} \
		string getCTypeName() const { \
			return #cname; \
		} \
	} idl##name;


#define ORBITCPP_MAKE_SIMPLE_INT_TYPE(name,cname) \
	static class IDL##name : public IDLSimpleType, public IDLUnionDescriminator { \
		string getTypeName() const { \
			return IDL_CORBA_NS "::" #name; \
		} \
		string getCTypeName() const { \
			return #cname; \
		} \
		virtual string getDefaultValue(set<string> const &labels) const { \
			short val=0; \
			string valstr; \
		  	do { \
				strstream ss; ss << val++ << ends; valstr = ss.str(); \
		  	} while( labels.find(valstr) != labels.end() ); \
			return valstr; \
		} \
	} idl##name;

#define ORBITCPP_MAKE_SIMPLE_CHAR_TYPE(name,cname) \
	static class IDL##name : public IDLSimpleType, public IDLUnionDescriminator { \
		string getTypeName() const { \
			return IDL_CORBA_NS "::" #name; \
		} \
		string getCTypeName() const { \
			return #cname; \
		} \
		virtual string getDefaultValue(set<string> const &labels) const { \
			return "\'\\0\'"; \
		} \
	} idl##name;


static class IDLBoolean : public IDLSimpleType, public IDLUnionDescriminator { \
	string getTypeName() const { 
		return IDL_CORBA_NS "::Boolean"; 
	} 
	string getCTypeName() const { 
		return "CORBA_boolean"; 
	} 
	virtual string getDefaultValue(set<string> const &labels) const {
		string val = "";
		if(labels.find("1") == labels.end()){
			val="1";
		} else if(labels.find("0") == labels.end()){
			val="0";
		}
		return val; 
	} 
} idlBoolean;


ORBITCPP_MAKE_SIMPLE_CHAR_TYPE(Char, CORBA_char)
ORBITCPP_MAKE_SIMPLE_CHAR_TYPE(WChar, CORBA_wchar)
ORBITCPP_MAKE_SIMPLE_TYPE(Octet, CORBA_octet)

ORBITCPP_MAKE_SIMPLE_INT_TYPE(Short, CORBA_short)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(UShort, CORBA_unsigned_short)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(Long, CORBA_long)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(ULong, CORBA_unsigned_long)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(LongLong, CORBA_long_long)
ORBITCPP_MAKE_SIMPLE_INT_TYPE(ULongLong, CORBA_unsigned_long_long)

ORBITCPP_MAKE_SIMPLE_TYPE(Float, CORBA_float)
ORBITCPP_MAKE_SIMPLE_TYPE(Double, CORBA_double)
ORBITCPP_MAKE_SIMPLE_TYPE(LongDouble, CORBA_long_double)

// IDLAny --------------------------------------------------
static class IDLAny : public IDLSimpleType {
	string getTypeName() const {
			return IDL_CORBA_NS "::Any";
	}
	string getCTypeName() const {
			return "CORBA_any";
	}
	virtual bool isVariableLength() const {
		return true;
	}
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = "const " + getTypeName() + "&";
		dcl = "_par_" + id;
	}
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << id << " = _par_" << id << ';' << endl;
	}
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef=NULL) const {
		switch (attr) {
		case IDL_PARAM_IN:
			typespec = "const " IDL_CORBA_NS "::Any";
			dcl = "&" + id;
			break;
		case IDL_PARAM_INOUT:
			typespec = IDL_CORBA_NS "::Any";
			dcl = "&" + id;
			break;
		case IDL_PARAM_OUT:
			typespec = IDL_CORBA_NS "::Any_out";
			dcl = id;
			break;
		}
	}
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		string retval;
		switch (attr) {
		case IDL_PARAM_IN:
		case IDL_PARAM_INOUT:
			retval = id + "._orbitcpp_get_c_any_ptr()";
			break;
		case IDL_PARAM_OUT:
			retval = idlGetCast("&" + id + ".ptr()", "CORBA_any**");
			break;
		}
		return retval;
	}

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = IDL_CORBA_NS "::Any";
		dcl = "*" + id;
	}
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "CORBA_any *_retval = NULL;" << endl;
	}
	string getCPPStubReturnAssignment() const {
		return "_retval = ";
	}
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "return " 
			<< idlGetCast("_retval", IDL_CORBA_NS "::Any*" ) << ";" << endl;
	}

	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const {
		switch( attr ) {
		case IDL_PARAM_IN:
			typespec = "const CORBA_any";
			dcl = "*" + id;
			break;
		case IDL_PARAM_INOUT:
			typespec = "CORBA_any";
			dcl = "*" + id;
			break;
		default:
			typespec = "CORBA_any";
			dcl = "**"+id;
		}
	}
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		switch(attr) {
		case IDL_PARAM_IN:
			return idlGetCast("*"+id, "const " IDL_CORBA_NS "::Any&");
		case IDL_PARAM_INOUT:
			return idlGetCast("*"+id, IDL_CORBA_NS "::Any&");
		default:
			return idlGetCast("*"+id, IDL_CORBA_NS "::Any*&");
		}
	}

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
		typespec = "CORBA_any";
		dcl = "*"+id;
	}
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << IDL_CORBA_NS "::Any *_retval = NULL;" << endl;
	}
	string getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
		return "_retval = ";
	}
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "return " 
			<< idlGetCast( "_retval", "CORBA_any*") << ";" << endl;
	}
	string getInvalidReturn() const {
		return "return NULL;";
	}
} idlAny;

// IDLArray --------------------------------------------------------------

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

bool
IDLArrayList::doesArrayTypeExist(IDLArray const& array) {
	string typespec, dcl="";
	array.m_elementType.getCPPMemberDeclarator(dcl,typespec,dcl);
	typespec += dcl;
	int length = 1;
	for(const_iterator it = array.begin(); it != array.end(); it++)
		length *= *it;
	IDLArrayKey new_array_key(typespec, length);
	if( m_arraySet.find(new_array_key) == m_arraySet.end() ) {
		m_arraySet.insert(new_array_key);
		return false;
	} else
		return true;
}

// IDLInterface --------------------------------------------------------------
bool 
IDLInterface::isBaseClass(IDLInterface *iface) {
	BaseList::const_iterator first = m_allbases.begin(),last = m_allbases.end();
	while (first != last)
		if (*first++ == iface) return true;
	return false;
}

bool
IDLInterface::requiresSmartPtr() const
{
	for(IDLInterface::BaseList::const_iterator it = m_allbases.begin(); it != m_allbases.end(); it++)
	{
		if( (*it)->m_all_mi_bases.begin() != (*it)->m_all_mi_bases.end() )
		{
			return true;
		}
	}

	return false;
}



void 
IDLInterface::writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
						   IDLElement &dest,IDLScope const &scope,
						   IDLTypedef const *activeTypedef = NULL) const {
	string id = dest.getCPPIdentifier();
	ostr
	<< indent << "typedef " << getCPPIdentifier() << ' ' << id << ';' << endl
	<< indent << "typedef " << getCPP_ptr() << ' ' << id << "_ptr" << ';' <<  endl
	<< indent << "typedef " << getCPP_mgr() << ' ' << id << "_mgr" << ';' <<  endl
	<< indent << "typedef " << getCPP_var() << ' ' << id << "_var" << ';' <<  endl
	<< indent << "typedef " << getCPP_out() << ' ' << id << "_out" << ';' <<  endl
	<< indent << "typedef " << getCPPIdentifier() << "Ref " << id << "Ref" << ';' <<  endl;

	// extra effort to typedef POA_ type
	string ns_outer_begin,ns_outer_end,ns_inner_begin,ns_inner_end;
	dest.getParentScope()->getCPPNamespaceDecl(ns_outer_begin,ns_outer_end);
	dest.getParentScope()->getCPPNamespaceDecl(ns_inner_begin,ns_inner_end,"POA_");

	ostr
	<< indent << ns_outer_end << ns_inner_begin << endl;
	indent++;
	ostr
	<< indent << "typedef " << getQualifiedCPP_POA() << ' ';
	if (dest.getParentScope() == getRootScope())
		ostr << "POA_";
	ostr << id << ';' << endl;
	
	// *** FIXME what about the _tie class?

	indent--;
	ostr
	<< indent << ns_inner_end << ns_outer_begin << endl;
}




void 
IDLInterface::writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
								      IDLTypedef const *activeTypedef = NULL) const {
	ostr
	<< indent << IDL_IMPL_NS "::release_guarded(_cstruct." << id << ");" << endl
	<< indent << "_cstruct." << id << " = "
	<< IDL_IMPL_NS "::duplicate_guarded(*" << id << ".in());" << endl;
}




void 
IDLInterface::writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
									    IDLTypedef const *activeTypedef = NULL) const {
	ostr
	<< id << " = " 
	<< getQualifiedCPPCast(IDL_IMPL_NS "::duplicate_guarded(_cstruct."+id+")")
	<< ';' << endl;
}




void 
IDLInterface::getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
								   IDLTypedef const *activeTypedef=NULL) const {
	dcl = id;

	switch (attr) {
	case IDL_PARAM_IN:
		typespec = getQualifiedCPP_ptr();
		break;
	case IDL_PARAM_INOUT:
		typespec = getQualifiedCPP_ptr();
		dcl = '&' + dcl;
		break;
	case IDL_PARAM_OUT:
		typespec = getQualifiedCPP_out();
		break;
	}
}



string 
IDLInterface::getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
									  IDLTypedef const *activeTypedef = NULL) const {
	string ctype = getNSScopedCTypeName();

	switch (attr) {
	case IDL_PARAM_IN:
		return "*"+id;
	case IDL_PARAM_INOUT:
		return "&reinterpret_cast< "+ctype+">("+id+")";
	case IDL_PARAM_OUT:
		return id;
	}
	return "";
}


void
IDLInterface::writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
											  IDLTypedef const *activeTypedef = NULL) const {
	// must return stub ptr and not ptr in order to work when smart pointers are used
	ostr
		<< indent << "return reinterpret_cast< " << getQualifiedCPPStub() << " *>(_retval);" << endl;
}



void 
IDLInterface::getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
								 IDLTypedef const *activeTypedef = NULL) const {
	typespec = getNSScopedCTypeName();

	switch (attr) {
	case IDL_PARAM_IN:
		dcl = id;
		break;
	case IDL_PARAM_INOUT:
		dcl = '*' + id;
		break;
	case IDL_PARAM_OUT:
		dcl = '*' + id;
		break;
	}
}




void 
IDLInterface::writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
										IDLTypedef const *activeTypedef = NULL) const {
	switch (attr) {
	case IDL_PARAM_IN:
		ostr
		<< indent << getQualifiedCPP_var() << " _" << id << "_ptr = "
		<< getQualifiedCPPCast(IDL_IMPL_NS "::duplicate_guarded("+id+")") 
		<< ';' << endl;
		break;
	case IDL_PARAM_INOUT:
		ostr
		<< indent << getQualifiedCPP_var() << " _" << id << "_ptr = "
		<< getQualifiedCPPCast(IDL_IMPL_NS "::duplicate_guarded(*"+id+")") 
		<< ';' << endl;
		break;
	case IDL_PARAM_OUT:
		ostr
		<< indent << getQualifiedCPP_var() << " _" << id << "_ptr = "
		<< getQualifiedCPPCast("CORBA_OBJECT_NIL") 
		<< ';' << endl;
		break;
	}
}




void 
IDLInterface::writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
									  IDLTypedef const *activeTypedef = NULL) const {
	string ptrname = " _" + id + "_ptr";
	switch (attr) {
	case IDL_PARAM_INOUT:
	case IDL_PARAM_OUT:
		ostr
		<< indent << '*' << id << " = *" << ptrname << "._retn();" << endl;
	default:
		break;
	}
}


void
IDLInterface::writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const {
	if (passthru)
		ostr << indent << "return _retval;" << endl;
	else {
		// this is a hack to ensure the cast works with MI smart ptrs	
		ostr << indent << "::CORBA::Object_ptr _tmp = _retval;" << endl;
		ostr << indent << "return reinterpret_cast< " << getNSScopedCTypeName() << ">(_tmp);" << endl;
	}
}




// IDLCompound ---------------------------------------------------------------
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




// Object type -------------------------------------------------------------
class IDLObject : public IDLInterface {
public:
	IDLObject()
		: IDLInterface("Object",NULL,NULL) {
	}

	string getCTypeName() const {
		return "CORBA_Object";
	}
	string getNSScopedCTypeName() const {
		return getCTypeName();
	}
	
	virtual string getQualifiedIDLIdentifier(IDLScope const *up_to = NULL,
											 IDLScope const *assumed_base = NULL) const {
		// fixme - doesn't do the scope thing
		return "CORBA::Object";
	}
	virtual string getQualifiedCIdentifier(IDLScope const *up_to = NULL,
										   IDLScope const *assumed_base = NULL) const {
		// fixme - doesn't do the scope thing
		return "CORBA_Object";
	}
	virtual string getQualifiedCPPIdentifier(IDLScope const *up_to = NULL,
											 IDLScope const *assumed_base = NULL) const {
		// fixme - doesn't do the scope thing
		return "CORBA::Object";
	}
	
	virtual string getQualifiedCPPStub(IDLScope const *up_to = NULL) const {
		// fixme - doesn't do the scope thing
		return "CORBA::Object";
	}


  virtual string getCPP_ptr() const {
		return "CORBA::Object_ptr";
	}
	virtual string getCPP_var() const {
		return "CORBA::Object_var";
	}
	virtual string getCPP_mgr() const {
		return "CORBA::Object_mgr";
	}
	virtual string getCPP_out() const {
		return "CORBA::Object_out";
	}

	virtual string getQualifiedCPP_ptr(IDLScope const *up_to = NULL) const {
		return "CORBA::Object_ptr";
	}
	virtual string getQualifiedCPP_var(IDLScope const *up_to = NULL) const {
		return "CORBA::Object_var";
	}
	virtual string getQualifiedCPP_mgr(IDLScope const *up_to = NULL) const {
		return "CORBA::Object_mgr";
	}
	virtual string getQualifiedCPP_out(IDLScope const *up_to = NULL) const {
		return "CORBA::Object_out";
	}

	
  
} idlObject;



class IDLTypeCode : public IDLInterface {
public:
	IDLTypeCode()
		: IDLInterface("TypeCode",NULL,NULL) {
	}

	string getCTypeName() const {
		return "CORBA_TypeCode";
	}
	string getNSScopedCTypeName() const {
		return getCTypeName();
	}
	
	virtual string getQualifiedIDLIdentifier(IDLScope const *up_to = NULL,
											 IDLScope const *assumed_base = NULL) const {
		// fixme - doesn't do the scope thing
		return "CORBA::TypeCode";
	}
	virtual string getQualifiedCIdentifier(IDLScope const *up_to = NULL,
										   IDLScope const *assumed_base = NULL) const {
		// fixme - doesn't do the scope thing
		return "CORBA_TypeCode";
	}
	virtual string getQualifiedCPPIdentifier(IDLScope const *up_to = NULL,
											 IDLScope const *assumed_base = NULL) const {
		// fixme - doesn't do the scope thing
		return "CORBA::TypeCode";
	}
	
	virtual string getQualifiedCPPStub(IDLScope const *up_to = NULL) const {
		// fixme - doesn't do the scope thing
		return "CORBA::TypeCode";
	}


  virtual string getCPP_ptr() const {
		return "CORBA::TypeCode_ptr";
	}
	virtual string getCPP_var() const {
		return "CORBA::TypeCode_var";
	}
	virtual string getCPP_mgr() const {
		return "CORBA::TypeCode_mgr";
	}
	virtual string getCPP_out() const {
		return "CORBA::TypeCode_out";
	}

	virtual string getQualifiedCPP_ptr(IDLScope const *up_to = NULL) const {
		return "CORBA::TypeCode_ptr";
	}
	virtual string getQualifiedCPP_var(IDLScope const *up_to = NULL) const {
		return "CORBA::TypeCode_var";
	}
	virtual string getQualifiedCPP_mgr(IDLScope const *up_to = NULL) const {
		return "CORBA::TypeCode_mgr";
	}
	virtual string getQualifiedCPP_out(IDLScope const *up_to = NULL) const {
		return "CORBA::TypeCode_out";
	}

	void
	writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
								  IDLTypedef const *activeTypedef = NULL) const {
		ostr << indent << "return reinterpret_cast< " << getNSScopedCTypeName() << ">(_retval);" << endl;		
	}

  
} idlTypeCode;




// IDLStruct ------------------------------------------------------------------

void
IDLStruct::writeInitCode(ostream &ostr, Indent &indent, string const &ident) const {
	IDLStruct::const_iterator first = begin(),last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeInitCode(ostr,indent,ident+"."+member.getCPPIdentifier());
	}
}

void IDLStruct::writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	IDLStruct::const_iterator first = begin(),last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeCPPDeepCopyCode(ostr,indent,ident+"."+member.getCPPIdentifier(),target+"."+member.getCPPIdentifier());
	}
}

void IDLStruct::writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {
	IDLStruct::const_iterator first = begin(),last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		member.getType()->writeCDeepCopyCode(ostr,indent,ident+"."+member.getCPPIdentifier(),target+"."+member.getCPPIdentifier());
	}
}

bool
IDLStruct::isVariableLength() const {
	IDLStruct::const_iterator first = begin(), last = end();
	while (first != last) {
		IDLMember &member = (IDLMember &) **first++;
		if(member.getType()->isVariableLength()){
			return true;
		}
	}
	return false;
}


void
IDLStruct::getCPPMemberDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
	typespec = getQualifiedCPPIdentifier(getRootScope());
	dcl = id;
};

void
IDLStruct::writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
						IDLElement &dest,IDLScope const &scope,
						IDLTypedef const *activeTypedef = NULL) const {
	ostr
	<< indent << "typedef " << getQualifiedCPPIdentifier()
	<< " " << dest.getCPPIdentifier() << ";" << endl;

	if(isVariableLength()){
	  	ostr
		<< indent << "typedef " << getQualifiedCPPIdentifier() 
		<< "_var " << dest.getCPPIdentifier() << "_var;" << endl;
	}

	ostr
	<< indent << "typedef " << getQualifiedCPPIdentifier() 
	<< "_out " << dest.getCPPIdentifier() << "_out;" << endl;
}


void
IDLStruct::getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									  IDLTypedef const *activeTypedef = NULL) const {
	ORBITCPP_NYI("struct getCPPStructCtorDeclarator");
}

void
IDLStruct::writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const {
	ORBITCPP_NYI("struct writeCPPStructCtor");
}

void
IDLStruct::writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const {
	ORBITCPP_NYI("struct writeCPPStructPacker");
}

void
IDLStruct::writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								  IDLTypedef const *activeTypedef = NULL) const {
	ORBITCPP_NYI("struct writeCPPStructUnpacker");
}


void
IDLStruct::writeUnionReferents(ostream &ostr,Indent &indent, string const &id, string const &discriminatorVal,
							IDLTypedef const *activeTypedef = NULL) const{
	ostr
	<< indent << getQualifiedCPPIdentifier() << " &" << id << "() {" << endl;
	ostr	
	<< ++indent << "return reinterpret_cast< " << getQualifiedCPPIdentifier()
	<< "&>(m_target._u." << id << ");" << endl;
	ostr
	<< --indent << "}" << endl;
}


void IDLStruct::getCPPStubDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
									 IDLTypedef const *activeTypedef=NULL) const {
	dcl = id;

	string name = activeTypedef ?
		activeTypedef->getQualifiedCPPIdentifier() : getQualifiedCPPIdentifier();
	switch (attr) {
	case IDL_PARAM_IN:
		typespec = "const " + name;
		dcl = '&' + dcl;
		break;
	case IDL_PARAM_INOUT:
		typespec = name;
		dcl = '&' + dcl;
		break;
	case IDL_PARAM_OUT:
		typespec = name + "_out";
		break;
	}
}

string 
IDLStruct::getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {

	string typespec,dcl;
	getCSkelDeclarator(attr,"",typespec,dcl,activeTypedef);

	switch (attr) {
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		return idlGetCast("&"+id,typespec+dcl);
	case IDL_PARAM_OUT:
		if(isVariableLength())
			return idlGetCast("&"+id+".ptr()",typespec+dcl);
		else
			return idlGetCast("&"+id,typespec+dcl);
	}
	return "";
}

void
IDLStruct::getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									  IDLTypedef const *activeTypedef = NULL) const {
	typespec = getQualifiedCPPIdentifier();
	if(isVariableLength())
		dcl = "*" + id;
	else
		dcl = id;
}

void
IDLStruct::writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									  IDLTypedef const *activeTypedef = NULL) const {

	ostr
	<< indent << getNSScopedCTypeName();
	if(isVariableLength())
		ostr << " *_retval = NULL;" << endl;
	else
		ostr << " _retval;" << endl;
}


string
IDLStruct::getCPPStubReturnAssignment() const {
	return "_retval = ";		
}

void
IDLStruct::writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										   IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << "return reinterpret_cast< " << getQualifiedCPPIdentifier();
	if(isVariableLength())
		ostr << "*";
	else
		ostr << "&";
	ostr << ">(_retval);" << endl;
}		


void 
IDLStruct::getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef = NULL) const {
	typespec = getNSScopedCTypeName();
	
	switch (attr) {
	case IDL_PARAM_IN:
		typespec = "const " + typespec;
		dcl = '*' + id;
		break;
	case IDL_PARAM_INOUT:
		dcl = '*' + id;
		break;
	case IDL_PARAM_OUT:
		if(isVariableLength())
			dcl = "**" + id;
		else
			dcl = '*' + id;
		break;
	}
}




void 
IDLStruct::writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
									 IDLTypedef const *activeTypedef = NULL) const {
	// no demarshalling code required
}




string
IDLStruct::getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
	string typespec,dcl;
	getCPPStubDeclarator(attr,"",typespec,dcl);

	switch (attr) {
	case IDL_PARAM_IN:
	case IDL_PARAM_INOUT:
		return idlGetCast("*"+id,typespec+dcl);
	case IDL_PARAM_OUT:
		if(isVariableLength())
			return idlGetCast("*"+id,getQualifiedCPPIdentifier()+"*&");
		else
			return idlGetCast("*"+id,typespec+dcl);
	}
	return "";
}



void 
IDLStruct::writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const {
	// no marshalling code required
}


void
IDLStruct::getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
	typespec = getNSScopedCTypeName();
	if (isVariableLength())
		dcl = "*" + id;
	else
		dcl = id;
}

void
IDLStruct::writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << getQualifiedCPPIdentifier();
	if (isVariableLength())
		ostr << " *_retval = NULL;" << endl;
	else
		ostr << " _retval;" << endl;
}

string
IDLStruct::getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
	return "_retval = ";		
}

void
IDLStruct::writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
										 IDLTypedef const *activeTypedef = NULL) const {
	ostr << indent << "return reinterpret_cast< "
	<< getNSScopedCTypeName();

	if(isVariableLength())
		ostr << "*";
	else
		ostr << "&";
	ostr
		<< ">(_retval);" << endl;
}


string
IDLStruct::getInvalidReturn() const {
	if(isVariableLength())
		return "return NULL;";
	else
		return "return reinterpret_cast< "
			+ getNSScopedCTypeName() + "&>(_retval);\n";
}




// IDLSequence -------------------------------------------------------------

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
	ostr << indent << ident+"._buffer = " << getNSScopedCTypeName()
		 << "_allocbuf("<< ident <<"._length);" << endl;
	
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
	strstream id;
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


// IDLEnum -------------------------------------------------------------
string
IDLEnum::getDefaultValue(set<string> const &labels)const {
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


// IDLUnion -------------------------------------------------------------
IDLUnion::IDLUnion(string const &id,IDL_tree node,
				   IDLType const &discriminatorType, IDLScope *parentscope = NULL)
	: IDLStruct(id,node,parentscope),m_discriminatorType(discriminatorType) {
}

bool
IDLUnion::isVariableLength() const {
	IDLUnion::const_iterator first = begin(), last = end();
	while (first != last) {
		IDLCaseStmt &stmt = (IDLCaseStmt &) **first++;
		const IDLMember &member = stmt.getMember();
		if(member.getType()->isVariableLength()){
			return true;
		}
	}
	return false;
}


string
IDLUnion::getDefaultDiscriminatorValue() const {
	IDLUnionDescriminator const &desc =
		dynamic_cast<IDLUnionDescriminator const &>(getDiscriminatorType());
	set<string> members;

	// collect all the union labels
	const_iterator it = begin();
	while (it != end()) {
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **it;
		IDLCaseStmt::const_iterator csit = casestmt.labelsBegin(),
			csend = casestmt.labelsEnd();
		while (csit != csend) {
			members.insert(*csit);
			csit++;
		}
		it++;
	}
	return desc.getDefaultValue(members);
}

bool
IDLUnion::hasExplicitDefault() const {
	bool result = false;
	const_iterator it = begin();
	while (it != end()) {
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **it;
		if(casestmt.isDefault()){
			result = true;
			break;
		}
		it++;
	}
	return result;
}


void IDLUnion::writeCPPDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {

	ostr << indent <<  "switch(" << target << "._d()) {" << endl;
	const_iterator it = begin();
	while (it != end()) {
		// collect the case labels for this member
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **it;
		IDLCaseStmt::const_iterator csit = casestmt.labelsBegin(),
			csend = casestmt.labelsEnd();
		if(casestmt.isDefault() == true) {
			ostr << indent << "default:" << endl;
		} else {
			while (csit != csend) {
				ostr << indent << "case " << *csit << ":" << endl;
				csit++;
			}
		}
		indent++;		  
		IDLMember const &member = casestmt.getMember();
		ostr << indent << ident << "." << member.getCPPIdentifier()
			 << "(" << target << "." << member.getCPPIdentifier() << "());" << endl;
		ostr << indent << "break;" << endl;
		it++;
		indent--;
	}
	ostr << indent << "}" << endl << endl;
}

void IDLUnion::writeCDeepCopyCode(ostream &ostr, Indent &indent, string const &ident,string const &target) const {

	ostr << indent <<  "switch(" << target << "._d) {" << endl;
	const_iterator it = begin();
	while (it != end()) {
		// collect the case labels for this member
		IDLCaseStmt &casestmt = (IDLCaseStmt &) **it;
		IDLCaseStmt::const_iterator csit = casestmt.labelsBegin(),
			csend = casestmt.labelsEnd();

		if(casestmt.isDefault() == true) {
			ostr << indent << "default:" << endl;
		} else {
			while (csit != csend) {
				ostr << indent << "case " << *csit << ":" << endl;
				csit++;
			}
		}
		
		indent++;		  
		IDLMember const &member = casestmt.getMember();
		IDLType const *elemtype = member.getType();
		elemtype->writeCDeepCopyCode(ostr,indent,ident+"._u."+member.getCIdentifier(),target+"._u."+member.getCIdentifier());	
		ostr << indent << "break;" << endl;
		it++;
		indent--;
	}

	if(hasExplicitDefault() == false) {
		ostr << indent++ << "default:" << endl;
		ostr << indent-- << "break;" << endl;
	}
	
	ostr << indent << "}" << endl << endl;
	ostr << indent << ident << "._d = " << target << "._d;" << endl;
}

// IDLTypeParser -------------------------------------------------------------
IDLTypeParser::~IDLTypeParser() {
	vector<IDLType *>::iterator
	first = m_anonymous_types.begin(),last = m_anonymous_types.end();

	while (first != last) delete *first++;
}




IDLType *
IDLTypeParser::parseTypeSpec(IDLScope &scope,IDL_tree typespec) {

	// I'm starting to think that this should be 2 methods:
	//  - one to parse the type, and one to parse the dcl into an id - PD
	// I noticed the same making some mods to the gather pass - JKL
	
	IDLType *type = NULL;

	if (typespec == NULL) type = &idlVoid;
	else
		switch (IDL_NODE_TYPE(typespec)) {
		case IDLN_TYPE_BOOLEAN:
			type = &idlBoolean;
			break;
		case IDLN_TYPE_CHAR:
			type = &idlChar;
			break;
		case IDLN_TYPE_WIDE_CHAR:
			type = &idlWChar;
			break;
		case IDLN_TYPE_OCTET:
			type = &idlOctet;
			break;
		case IDLN_TYPE_STRING:
			type = &idlString;
			break;
		case IDLN_TYPE_ANY:
			type = &idlAny;
			break;
		case IDLN_TYPE_OBJECT:
			type = &idlObject;
			break;
		case IDLN_TYPE_TYPECODE:
			type=&idlTypeCode;
			break;
				
		case IDLN_TYPE_INTEGER:
			if (IDL_TYPE_INTEGER(typespec).f_signed) {
				switch (IDL_TYPE_INTEGER(typespec).f_type) {
				case IDL_INTEGER_TYPE_SHORT:
					type = &idlShort;
					break;
				case IDL_INTEGER_TYPE_LONG:
					type = &idlLong;
					break;
				case IDL_INTEGER_TYPE_LONGLONG:
					type = &idlLongLong;
					break;
				}
			} else {
				switch (IDL_TYPE_INTEGER(typespec).f_type) {
				case IDL_INTEGER_TYPE_SHORT:
					type = &idlUShort;
					break;
				case IDL_INTEGER_TYPE_LONG:
					type = &idlULong;
					break;
				case IDL_INTEGER_TYPE_LONGLONG:
					type = &idlULongLong;
					break;
				}
			}
			break;

		case IDLN_TYPE_FLOAT:
			switch (IDL_TYPE_FLOAT(typespec).f_type) {
			case IDL_FLOAT_TYPE_FLOAT:
				type = &idlFloat;
				break;
			case IDL_FLOAT_TYPE_DOUBLE:
				type = &idlDouble;
				break;
			case IDL_FLOAT_TYPE_LONGDOUBLE:
				type = &idlLongDouble;
				break;
			}
			break;

		case IDLN_IDENT:
			{
				IDLElement *item = scope.lookup(idlGetQualIdentifier(typespec));
				if (!item) throw IDLExUnknownIdentifier(typespec,idlGetQualIdentifier(typespec));
				if (!item->isType()) throw IDLExTypeIdentifierExpected(typespec,IDL_IDENT(typespec).str);
				type = dynamic_cast<IDLType *>(item);
				break;
			}
		case IDLN_TYPE_SEQUENCE:
			{
				// parse the sequence element type

      	//TODO: Sequences need to be fixed. murrayc. See
				//http://lists.gnome.org/archives/orbit-list/2002-February/msg00131.html
				IDLType *type_seq = parseTypeSpec(scope,IDL_TYPE_SEQUENCE(typespec).simple_type_spec);
				IDLSequence* seq = 0;
				if (IDL_TYPE_SEQUENCE(typespec).positive_int_const == NULL){
					seq = new IDLSequence(*type_seq,0);
				} else {
					string len_str =
						idlTranslateConstant(IDL_TYPE_SEQUENCE(typespec).positive_int_const,scope);
					int length = atoi(len_str.c_str());
					seq = new IDLSequence(*type_seq,length);
				}
				m_anonymous_types.push_back(seq);
				type = seq;
				break;
			}
		case IDLN_TYPE_ARRAY:
		    {
				cout << "Array!";
				break;
			}
		ORBITCPP_DEFAULT_CASE(typespec)
		}

	if (!type) ORBITCPP_NYI(idlGetNodeTypeString(typespec))

	return type;
}

IDLType*
IDLTypeParser::parseDcl(IDL_tree dcl, IDLType *typespec, string &id)
{
	IDLType *ret_type = typespec;
	
	if (IDL_NODE_TYPE(dcl) == IDLN_IDENT){
		id = IDL_IDENT(dcl).str;
	} else if (IDL_NODE_TYPE(dcl) == IDLN_TYPE_ARRAY) {
		ret_type = new IDLArray(*typespec,
								  IDL_IDENT(IDL_TYPE_ARRAY(dcl).ident).str,
								  dcl);
		// this anon_types name is obviously an "artifact" from MICO, but the behavior
		// is what we want (the memory is freed upon destruction of the IDLTypeParser)
		m_anonymous_types.push_back(ret_type);
		id = IDL_IDENT(IDL_TYPE_ARRAY(dcl).ident).str;
	} else 
		ORBITCPP_NYI(" declarators:"+idlGetNodeTypeString(dcl));
	return ret_type;
}
