/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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


#ifndef ORBITCPP_TYPES_IDLINTERFACE
#define ORBITCPP_TYPES_IDLINTERFACE

#include "IDLUserDefScopeType.hh"

class IDLInterface : public IDLUserDefScopeType {
public:
	typedef vector<IDLInterface *> 		BaseList;
	BaseList 							m_bases;
	// all bases except the ones stemming from the first direct base
	BaseList 							m_all_mi_bases;
	BaseList 							m_allbases;

	IDLInterface(string const &id,IDL_tree node,IDLScope *parentscope = NULL)
		: IDLUserDefScopeType(id,node,parentscope) {
	}

	bool isVariableLength() const {
		return true;
	}
	bool isBaseClass(IDLInterface *iface);

  	virtual string getCPPStub() const {
		//return getCPPIdentifier()+"_stub";
		return getCPPIdentifier();
	}

	string getCPP_ptr() const {
		return getCPPIdentifier()+"_ptr";
	}
	string getCPP_var() const {
		return getCPPIdentifier()+"_var";
	}
	string getCPP_mgr() const {
		return getCPPIdentifier()+"_mgr";
	}
	string getCPP_out() const {
		return getCPPIdentifier()+"_out";
	}
	string getCPP_POA() {
		if (getParentScope() == getRootScope()) return "POA_"+getCPPIdentifier();
		else return getCPPIdentifier();
	}
	string getCPP_tie() {
		return getCPP_POA() + "_tie";
	}
	virtual string getQualifiedCPPStub(IDLScope const *up_to = NULL) const {
		// This is a dodgy hack - fixme!
		string retval;
		if(up_to == NULL)
			retval = IDL_IMPL_STUB_NS + getQualifiedCPPIdentifier();
		else if (up_to == getRootScope())
			retval = IDL_IMPL_NS_ID "::" IDL_IMPL_STUB_NS_ID  + getQualifiedCPPIdentifier();
		else
			g_error("getQualifiedCPP_stub doesnt support an up_to unless it's rootScope");
		return retval;
	}

	string getQualifiedCPP_ptr(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_ptr";
	}
	string getQualifiedCPP_var(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_var";
	}
	string getQualifiedCPP_mgr(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_mgr";
	}
	string getQualifiedCPP_out(IDLScope const *up_to = NULL) const {
		return getQualifiedCPPIdentifier(up_to) + "_out";
	}

	string getQualifiedCPPCast(string const &expr) const;

	// cast with ref for smart pointer to normal ptr
	string getQualifiedCPPSmartptrCast(string const &expr) const {
		return "reinterpret_cast< "+getQualifiedCPP_ptr()+"&>("+expr+")";
	}

	bool requiresSmartPtr() const;

	string getQualifiedCPP_POA() const {
		return "POA_" + getQualifiedCPPIdentifier(getRootScope());
	}
	string getQualifiedC_POA() const {
		return "POA_" + getQualifiedCIdentifier();
	}
	string getQualifiedC_epv() const {
		return "POA_" + getQualifiedCIdentifier()+"__epv";
	}
	string getQualifiedC_vepv() const {
		return "POA_" + getQualifiedCIdentifier()+"__vepv";
	}
	string getRepositoryId() const {
		return IDL_IDENT_REPO_ID(IDL_INTERFACE(getNode()).ident);
	};

	void getCPPpoaNamespaceDecl(string &ns_begin,string &ns_end) const {
		getParentScope()->getCPPNamespaceDecl(ns_begin,ns_end,"POA_");
	}
	string getCPPpoaIdentifier() const {
		if (getParentScope() == getRootScope()) return "POA_"+getCPPIdentifier();
		else return getCPPIdentifier();
	}

	// misc stuff
	void getCPPMemberDeclarator (string const     &id,
				     string           &typespec,
				     string           &dcl,
				     IDLTypedef const *activeTypedef = NULL) const;
    
	void writeTypedef(ostream &ostr,Indent &indent,IDLCompilerState &state,
					  IDLElement &dest,IDLScope const &scope,
					  IDLTypedef const *activeTypedef = NULL) const;

	// Container accessors
	string getForwarder () const;
	void writeForwarder (ostream &header_ostr,
			     Indent  &header_indent,
			     ostream &impl_ostr,
			     Indent  &impl_indent) const;
	
	// struct / exception stuff
	void getCPPStructCtorDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = getQualifiedCPP_ptr();
		dcl = "_par_"+id;
	}
	void writeCPPStructCtor(ostream &ostr,Indent &indent,string const &id,
							IDLTypedef const *activeTypedef = NULL) const {
		ostr
			<< indent << id << " = " << getQualifiedCPPCast("_par_"+id)
			<< ';' << endl;
	}
	void writeCPPStructPacker(ostream &ostr,Indent &indent,string const &id,
							  IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPStructUnpacker(ostream &ostr,Indent &indent,string const &id,
								IDLTypedef const *activeTypedef = NULL) const;

	// sequence stuff
	string getCTypeName() const {
		return  getQualifiedCIdentifier();
	}
	
	// stub stuff
	void getCPPStubDeclarator(IDL_param_attr attr,string const &id,
							  string &typespec,string &dcl,
							  IDLTypedef const *activeTypedef = NULL) const;
	string getCPPStubParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const;

	// stub return stuff
	void getCPPStubReturnDeclarator(string const &id,string &typespec,string &dcl,
									IDLTypedef const *activeTypedef = NULL) const {
		typespec = getQualifiedCPP_ptr();
		dcl = id;
	}
	void writeCPPStubReturnPrepCode(ostream &ostr,Indent &indent,
									IDLTypedef const *activeTypedef = NULL) const {
		ostr
			<< indent << getNSScopedCTypeName()
			<< " _retval = NULL;" << endl;
	}
	string getCPPStubReturnAssignment() const {
		return "_retval = ";
	}
	void writeCPPStubReturnDemarshalCode(ostream &ostr,Indent &indent,
										 IDLTypedef const *activeTypedef = NULL) const;
	// skel stuff
	void getCSkelDeclarator(IDL_param_attr attr,string const &id,string &typespec,string &dcl,
							IDLTypedef const *activeTypedef = NULL) const;
	void writeCPPSkelDemarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								   IDLTypedef const *activeTypedef = NULL) const;
	string getCPPSkelParameterTerm(IDL_param_attr attr,string const &id,
								   IDLTypedef const *activeTypedef = NULL) const {
		return "_" + id + "_ptr";
	}
	void writeCPPSkelMarshalCode(IDL_param_attr attr,string const &id,ostream &ostr,Indent &indent,
								 IDLTypedef const *activeTypedef = NULL) const;

	// skel return stuff
	void getCSkelReturnDeclarator(string const &id,string &typespec,string &dcl,
								  IDLTypedef const *activeTypedef = NULL) const {
		typespec = this->getNSScopedCTypeName();
		dcl = id;
	}
	void writeCPPSkelReturnPrepCode(ostream &ostr,Indent &indent,bool passthru,
									
									IDLTypedef const *activeTypedef = NULL) const {
		if (passthru)
			ostr << indent << getQualifiedCIdentifier() << " _retval;" << endl;
		else
			ostr << indent << getQualifiedCPP_ptr()
				<< " _retval = NULL;" << endl;
	}
	string getCPPSkelReturnAssignment(bool passthru,
									  IDLTypedef const *activeTypedef = NULL) const {
		return "_retval = ";
	}
	void writeCPPSkelReturnMarshalCode(ostream &ostr,Indent &indent,bool passthru,
									   IDLTypedef const *activeTypedef = NULL) const;

	string getInvalidReturn() const {
		return "return NULL;";
	}
};

#endif //ORBITCPP_TYPES_IDLINTERFACE

