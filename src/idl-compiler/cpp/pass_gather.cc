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
 *
 *  Purpose: information gathering pass
 *
 */




#include "error.hh"
#include "pass_gather.hh"




// IDLPassGather -------------------------------------------------------------
void 
IDLPassGather::runPass() {
	doDefinitionList(m_state.m_rootscope.getNode(),m_state.m_rootscope);
	runJobs();
}




void 
IDLPassGather::doTypedef(IDL_tree node,IDLScope &scope) {
	string id;

	IDL_tree dcl_list = IDL_TYPE_DCL(node).dcls;
	IDLType *type_spec = m_state.m_typeparser.parseTypeSpec(
		                    scope,IDL_TYPE_DCL(node).type_spec);
	
	while (dcl_list) {
		IDLType *type = m_state.m_typeparser.parseDcl(
		                     IDL_LIST(dcl_list).data,type_spec,id);
		IDLTypedef *td = new IDLTypedef(*type,id,IDL_LIST(dcl_list).data,&scope);
		ORBITCPP_MEMCHECK(td)

		dcl_list = IDL_LIST(dcl_list).next;
	}
}




void 
IDLPassGather::doStruct(IDL_tree node,IDLScope &scope) {
	IDLStruct *idlStruct = new IDLStruct(
		IDL_IDENT(IDL_TYPE_STRUCT(node).ident).str,node,&scope
	);
	ORBITCPP_MEMCHECK(idlStruct);

	Super::doStruct(node,*idlStruct);
}





void 
IDLPassGather::doUnion(IDL_tree node,IDLScope &scope) {

	IDLType *type = m_state.m_typeparser.parseTypeSpec(scope,
		IDL_TYPE_UNION(node).switch_type_spec);
	
	IDLUnion *idlUnion = new IDLUnion(
		IDL_IDENT(IDL_TYPE_UNION(node).ident).str,node,*type,&scope
	);
	ORBITCPP_MEMCHECK(idlUnion);
	Super::doUnion(node,*idlUnion);
}




void 
IDLPassGather::doEnum(IDL_tree node,IDLScope &scope) {
	IDLEnum *enm = new IDLEnum(IDL_IDENT(IDL_TYPE_ENUM(node).ident).str,node,&scope);
	ORBITCPP_MEMCHECK(enm)
}




void 
IDLPassGather::doNative(IDL_tree node,IDLScope &scope) {
	ORBITCPP_NYI("native")
}




void 
IDLPassGather::doConstant(IDL_tree node,IDLScope &scope) {
	string id;
	IDLType *type = m_state.m_typeparser.parseTypeSpec(scope,
		IDL_CONST_DCL(node).const_type);
	type = m_state.m_typeparser.parseDcl(IDL_CONST_DCL(node).ident,type,id);
	
	ORBITCPP_MEMCHECK(
	 	new IDLConstant(type,id,node,&scope)
		);
}




void 
IDLPassGather::doAttribute(IDL_tree node,IDLScope &scope) {
	string id;

	IDLType *type = m_state.m_typeparser.parseTypeSpec(scope,IDL_ATTR_DCL(node).param_type_spec);

	for(IDL_tree item = IDL_ATTR_DCL(node).simple_declarations;
		item; item = IDL_LIST(item).next)  {

		IDL_tree dcl = IDL_LIST(item).data;
		IDLType *type_dcl = m_state.m_typeparser.parseDcl(dcl, type, id);
		
		new IDLAttribute(id,node,type_dcl,&scope);
	}
}




void 
IDLPassGather::doOperation(IDL_tree node,IDLScope &scope) {
	string id;
	IDLType *ret_type = m_state.m_typeparser.parseTypeSpec(scope,IDL_OP_DCL(node).op_type_spec);
	ret_type = m_state.m_typeparser.parseDcl(IDL_OP_DCL(node).ident,ret_type,id);


	IDLOperation *op = new IDLOperation(id,node,&scope);
	ORBITCPP_MEMCHECK(op);

	op->m_returntype = ret_type;

	IDL_tree parlist = IDL_OP_DCL(node).parameter_dcls;

	while (parlist) {
		IDLOperation::ParameterInfo pi;
		pi.Direction = IDL_PARAM_DCL(IDL_LIST(parlist).data).attr;
		pi.Type = m_state.m_typeparser.parseTypeSpec(scope,
		                                 IDL_PARAM_DCL(IDL_LIST(parlist).data).param_type_spec);
		pi.Type = m_state.m_typeparser.parseDcl(
		                                 IDL_PARAM_DCL(IDL_LIST(parlist).data).simple_declarator,
		                                 pi.Type, pi.Identifier);


		op->m_parameterinfo.push_back(pi);

		parlist = IDL_LIST(parlist).next;
	}


	IDL_tree raises_list = IDL_OP_DCL(node).raises_expr;
	while (raises_list) {
		IDLException *ex = (IDLException *) scope.lookup(
		                        idlGetQualIdentifier(IDL_LIST(raises_list).data));

		op->m_raises.push_back(ex);

		raises_list = IDL_LIST(raises_list).next;
	}
}




void 
IDLPassGather::doMember(IDL_tree node,IDLScope &scope) {
	string id;

	IDLType *type_spec = m_state.m_typeparser.parseTypeSpec(scope,
	                         IDL_TYPE_DCL(node).type_spec);

	IDL_tree dcl_list = IDL_MEMBER(node).dcls;
	while (dcl_list) {
		IDLType *type = m_state.m_typeparser.parseDcl(IDL_LIST(dcl_list).data,type_spec,id);
		
		IDLElement *member = new IDLMember(type,id,IDL_LIST(dcl_list).data,&scope);
		ORBITCPP_MEMCHECK(member)

		dcl_list = IDL_LIST(dcl_list).next;
	}
}

void 
IDLPassGather::doCaseStmt(IDL_tree node,IDLScope &scope) {
	string id;

	// member
	IDL_tree member = IDL_CASE_STMT(node).element_spec;
	g_assert(IDL_NODE_TYPE(member) == IDLN_MEMBER);
	IDL_tree dcl = IDL_LIST(IDL_MEMBER(member).dcls).data;   // dont handle multiple dcls	
	g_assert(IDL_NODE_TYPE(dcl) == IDLN_IDENT);
	IDLType *type = m_state.m_typeparser.parseTypeSpec(scope,IDL_TYPE_DCL(member).type_spec);
	type = m_state.m_typeparser.parseDcl(dcl,type,id);
	IDLMember *themember = new IDLMember(type,id,dcl);  // don't attach this to the scope
	new IDLCaseStmt(themember,id,node,&scope);  // attach the case stmt instead
	// case stmt takes ownership of member
}



void 
IDLPassGather::doException(IDL_tree node,IDLScope &scope) {
	IDLException *except = new IDLException(
	                            IDL_IDENT(IDL_EXCEPT_DCL(node).ident).str,node,&scope
	                        );
	ORBITCPP_MEMCHECK(except);

	Super::doException(node,*except);
}




void 
IDLPassGather::doInterface(IDL_tree node,IDLScope &scope) {
	string ident = IDL_IDENT(IDL_INTERFACE(node).ident).str;
	IDLInterface *iface = new IDLInterface(ident,node,&scope);
	ORBITCPP_MEMCHECK(iface)
	m_state.m_interfaces.push_back(iface);

	enumerateBases(*iface,false,NULL,iface->m_bases);
	IDLInterface *firstbase = NULL;
	if (iface->m_bases.size()) firstbase = iface->m_bases[0];
	enumerateBases(*iface,true,firstbase,iface->m_all_mi_bases);
	enumerateBases(*iface,true,NULL,iface->m_allbases);

	Super::doInterface(node,*iface);
}




void
IDLPassGather::doForwardDcl(IDL_tree node,IDLScope &scope) {
	string ident = IDL_IDENT(IDL_INTERFACE(node).ident).str;
	IDLInterface *iface = new IDLInterface(ident,node,&scope);
	ORBITCPP_MEMCHECK(iface)
	// Don't put this interfaces into the m_state.m_interfaces, or the declaration
	// will be written twice!
	// m_state.m_interfaces.push_back(iface);
}




void 
IDLPassGather::doModule(IDL_tree node,IDLScope &scope) {
	IDLScope *module = new IDLScope(
	                        IDL_IDENT(IDL_MODULE(node).ident).str,node,&scope
	                    );
	ORBITCPP_MEMCHECK(module)

	Super::doModule(node,*module);
}




void 
IDLPassGather::enumerateBases(IDLInterface &iface,bool recurse,IDLInterface *omit_with_bases,
                                    vector<IDLInterface *> &dest) {
	IDL_tree inh_list = IDL_INTERFACE(iface.getNode()).inheritance_spec;
	if (!inh_list) return;

	while (inh_list) {
		string base_id = idlGetQualIdentifier(IDL_LIST(inh_list).data);
		IDLInterface *base = (IDLInterface *) iface.getParentScope()->lookup(base_id);
		if (!base) throw IDLExUnknownIdentifier(IDL_LIST(inh_list).data,base_id);

		if (recurse) enumerateBases(*base,true,omit_with_bases,dest);

		vector<IDLInterface *>::const_iterator
		first = dest.begin(),last = dest.end();

		bool already_in = false, omit = false;
		while (first != last && !already_in)
			if (*first++ == base) already_in = true;
		
		if (omit_with_bases) {
			omit = base == omit_with_bases || omit_with_bases->isBaseClass(base);
		}
		
		if (!already_in && !omit) dest.push_back(base);

		inh_list = IDL_LIST(inh_list).next;
	}
}
