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
 *          Phil Dawes        <philipd@users.sourceforge.net>
 *
 *  Purpose:	IDL compiler generic IDL iterating pass
 *
 */




#include "pass_idl_it.hh"
#include "error.hh"




// IDLIteratingPass ----------------------------------------------------------
void IDLIteratingPass::doException(IDL_tree node,IDLScope &scope) {
	doMemberList(IDL_EXCEPT_DCL(node).members,scope);
}

void IDLIteratingPass::doTypedef(IDL_tree node,IDLScope &scope)
{
	handle_node (IDL_TYPE_DCL (node).type_spec, scope);
}


void IDLIteratingPass::doStruct(IDL_tree node, IDLScope &scope) {
	doMemberList(IDL_TYPE_STRUCT(node).member_list,scope);
}

void IDLIteratingPass::doUnion(IDL_tree node, IDLScope &scope) {
	doSwitchBody(IDL_TYPE_UNION(node).switch_body,scope);
}


void IDLIteratingPass::doInterface(IDL_tree node,IDLScope &scope) {
	doExportList(IDL_INTERFACE(node).body,scope);
}




void IDLIteratingPass::doModule(IDL_tree node,IDLScope &scope) {
	doDefinitionList(IDL_MODULE(node).definition_list,scope);
}




void IDLIteratingPass::doMemberList(IDL_tree  member_list,
				    IDLScope &scope)
{
	while (member_list) {
		enumHook (member_list, scope);

		IDL_tree      member = IDL_LIST (member_list).data;
		IDL_tree_type member_type = IDL_NODE_TYPE (member);
		
		switch (member_type) {
		case IDLN_MEMBER:
			doMember (member, scope);
			break;
		ORBITCPP_DEFAULT_CASE (member)
		}
		
		member_list = IDL_LIST (member_list).next;
	}

	enumHook (member_list, scope);
}

void IDLIteratingPass::doSwitchBody(IDL_tree  member_list,
				    IDLScope &scope)
{
	while (member_list) {
		enumHook (member_list, scope);
		
		IDL_tree casestmt = IDL_LIST (member_list).data;
		g_assert (IDL_NODE_TYPE (casestmt) == IDLN_CASE_STMT);

		doCaseStmt (casestmt, scope);

		member_list = IDL_LIST (member_list).next;
	}
	
	enumHook (member_list, scope);
}


void IDLIteratingPass::handle_node (IDL_tree node, IDLScope &scope)
{
	switch (IDL_NODE_TYPE(node)) {
	case IDLN_SRCFILE:
		// Do nothing - otherwise we'd throw an unxpected node exception.
		// What does IDLN_SRCFILE actually mean? We seem to get
		// it as the first node, so maybe it's like a root node.
		// murrayc.
		break;
	case IDLN_CODEFRAG:
		//Ignore this unless we think of some need for it. murrayc.
		break;
		
	case IDLN_INTERFACE:
		doInterface (node, scope);
		break;
	case IDLN_MODULE:
		doModule (node, scope);
		break;
		
	case IDLN_FORWARD_DCL:
		doForwardDcl (node, scope);
		break;
 	case IDLN_TYPE_DCL:
		doTypedef (node, scope);
		break;
	case IDLN_CONST_DCL:
		doConstant (node, scope);
		break;
		
	case IDLN_TYPE_ENUM:
		doEnum (node, scope);
		break;
	case IDLN_TYPE_STRUCT:
		doStruct (node, scope);
		break;
	case IDLN_TYPE_SEQUENCE:
		doSequence (node, scope);
		break;
	case IDLN_TYPE_UNION:
		doUnion (node, scope);
		break;
#if 0 //!!!
	case IDLN_NATIVE:
		doNative (node, scope);
		break;			
#endif
	case IDLN_ATTR_DCL:
		doAttribute (node, scope);
		break;
	case IDLN_EXCEPT_DCL:
		doException (node, scope);
		break;
	case IDLN_OP_DCL:
		doOperation (node, scope);
		break;
	ORBITCPP_DEFAULT_CASE(node);
	}
}		
	
void IDLIteratingPass::doExportList(IDL_tree list,IDLScope &scope) {
	while (list) {
		enumHook(list,scope);
		handle_node (IDL_LIST (list).data, scope);
		list = IDL_LIST(list).next;
	}
	enumHook(list,scope);
}




void IDLIteratingPass::doDefinitionList(IDL_tree list,IDLScope &scope) {
	ORBITCPP_EXPECT_TYPE(list,IDLN_LIST)

	while (list) {
		enumHook(list,scope);
		handle_node (IDL_LIST (list).data, scope);
		list = IDL_LIST(list).next;
	}

	enumHook(list,scope);
}
