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
 *  Purpose: skeleton generation pass
 *
 */




#include "error.hh"
#include "pass_skels.hh"

#include <iostream>
#include <sstream>


// IDLPassSkels --------------------------------------------------------------
void 
IDLPassSkels::runPass() {
	m_header
	<< indent << "#ifndef __ORBITCPP_IDL_" << idlUpper(m_state.m_basename) << "_SKELS" << endl
	<< indent << "#define __ORBITCPP_IDL_" << idlUpper(m_state.m_basename) << "_SKELS" << endl
	<< indent << "#include \"" << m_state.m_basename << IDL_CPP_STUB_HEADER_EXT"\"" << endl
	<< indent << endl << endl
	<< indent << "// Skeleton declaration --------------------------------" << endl
	<< indent << endl;

	m_module
	<< mod_indent << "#include \"" << m_state.m_basename << IDL_CPP_SKEL_HEADER_EXT"\"" << endl
	<< mod_indent << endl << endl
	<< mod_indent << "// Skeleton code ---------------------------------------" << endl
	<< mod_indent << endl;

	vector<IDLInterface *>::iterator
	first = m_state.m_interfaces.begin(), last = m_state.m_interfaces.end();
	while (first != last)
		doInterface(*(*first++));
	
	runJobs();
	
	m_header << endl << indent << "#endif" << endl;
}




// operation-scope methods ----------------------------------------------------
#if 0 //!!!
void 
IDLPassSkels::doAttributeSkelPrototype(IDLInterface &iface,IDLInterface &of,IDL_tree node) {
	IDLAttribute &attr = (IDLAttribute &) *of.getItem(node);

	// get	
	string ret_typespec,ret_typedcl;
	attr.getType()->getCSkelReturnDeclarator("_skel__get_" + attr.getCPPIdentifier(),ret_typespec,ret_typedcl);
	
	m_header
	<< indent << "static " << ret_typespec << ' ' << ret_typedcl << '('
	<<  "::PortableServer_Servant _servant,::CORBA_Environment *_ev);" << endl;

	// set
	if(!attr.isReadOnly()){
		string type,dcl;
		attr.getType()->getCSkelDeclarator(IDL_PARAM_IN,"val",type,dcl);

		m_header
		 <<	indent << "static void _skel__set_" << attr.getCPPIdentifier()
		 << "(::PortableServer_Servant _servant, " << type << ' ' << dcl << ", ::CORBA_Environment *_ev);" << endl;
		
	}	
}
#endif




void 
IDLPassSkels::doOperationSkelPrototype (IDLInterface &iface,
					IDLInterface &of,
					IDL_tree      node)
{
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	m_header << indent << "static "
		 << op.skel_decl_proto ()
		 << ";" << endl;
	
	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}



#if 0 //!!!
void 
IDLPassSkels::doAttributeSkel(IDLInterface &iface,IDLInterface &of,IDL_tree node) {
	IDLAttribute &attr = (IDLAttribute &) *of.getItem(node);

	// ----------------- get method ---------------------
	string ret_typespec,ret_typedcl;
	attr.getType()->getCSkelReturnDeclarator(
	    iface.getQualifiedCPP_POA() + "::_skel__get_" + attr.getCPPIdentifier(),
	    ret_typespec,ret_typedcl
	);
	m_module
	<< mod_indent << ret_typespec << ' ' << ret_typedcl << '('
	<<  "::PortableServer_Servant _servant, ::CORBA_Environment *_ev) {" << endl;
	mod_indent++;

	// inherited => call up base "passthru"
	if (&iface != &of) {
		m_module
		<< mod_indent << of.getQualifiedCPP_POA() << "::_orbitcpp_Servant _fake;" << endl
		<< mod_indent << "_fake.m_cppimpl = ((_orbitcpp_Servant *)_servant)->m_cppimpl; // causes implicit cast" << endl;
		
		m_module
		<< mod_indent << "return "
		<< of.getQualifiedCPP_POA() << "::_skel__get_" << attr.getCPPIdentifier()
		<< "(&_fake,_ev);" << endl;
	  
	} else {

		attr.getType()->writeCPPSkelReturnPrepCode(m_module,mod_indent);
		m_module
			<< mod_indent << "bool _results_valid = true;" << endl << endl;

	  // do the call
		m_module
			<< mod_indent << "try {" << endl;
		mod_indent++;
		m_module
		<< mod_indent << iface.getQualifiedCPP_POA() << " *_self = "
		<< "((_orbitcpp_Servant *)_servant)->m_cppimpl;" << endl
		<< mod_indent << attr.getType()->getCPPSkelReturnAssignment()
		<< "_self->" << attr.getCPPIdentifier() << "();" << endl;
		m_module << --mod_indent << "}" << endl;
		// handle exceptions
		m_module
		<< mod_indent << "catch ("IDL_CORBA_NS "::Exception &_ex) {" << endl;
		mod_indent++;
		m_module
		<< mod_indent << "_results_valid = false;" << endl
		<< mod_indent << "_ex._orbitcpp_set(_ev);" << endl;
		m_module
		<< --mod_indent << '}' << endl;

		attr.getType()->writeCPPSkelReturnMarshalCode(m_module,mod_indent);
	}
	// end function
	m_module
	  << --mod_indent << '}' << endl << endl;
	
	// ----------------- set method ---------------------
	if(!attr.isReadOnly()){
		string type,dcl;
		attr.getType()->getCSkelDeclarator(IDL_PARAM_IN,"val",type,dcl);

		m_module
		<< "void "
		<< iface.getQualifiedCPP_POA() << "::_skel__set_" + attr.getCPPIdentifier()
		<< "(::PortableServer_Servant _servant, " << type << ' '
		<< dcl << ", ::CORBA_Environment *_ev) { " << endl;
		mod_indent++;

		// inherited => call up base "passthru"
		if (&iface != &of) {
			m_module
			<< mod_indent << of.getQualifiedCPP_POA() << "::_orbitcpp_Servant _fake;" << endl
			<< mod_indent << "_fake.m_cppimpl = ((_orbitcpp_Servant *)_servant)->m_cppimpl; // causes implicit cast" << endl;
		
			m_module
			<< mod_indent << of.getQualifiedCPP_POA()
			<< "::_skel__set_" << attr.getCPPIdentifier()
			<< "(&_fake,val,_ev);" << endl;			
			
		} else {	  		
			m_module << mod_indent << "bool _results_valid = true;" << endl << endl;

		
			attr.getType()->writeCPPSkelDemarshalCode(IDL_PARAM_IN,"val",m_module,mod_indent);

		// do the call
			m_module
			<< mod_indent << "try {" << endl;
			mod_indent++;
			m_module
			<< mod_indent << iface.getQualifiedCPP_POA() << " *_self = "
			<< "((_orbitcpp_Servant *)_servant)->m_cppimpl;" << endl;

			m_module
			<< mod_indent << "_self->" << attr.getCPPIdentifier() << '('
			<< attr.getType()->getCPPSkelParameterTerm(IDL_PARAM_IN,"val")
			<< ");" << endl
			<< --mod_indent << '}' << endl;
			// handle exceptions
			m_module
			<< mod_indent << "catch ("IDL_CORBA_NS "::Exception &_ex) {" << endl;
			mod_indent++;
			m_module
			<< mod_indent << "_results_valid = false;" << endl
			<< mod_indent << "_ex._orbitcpp_set(_ev);" << endl;
			m_module
			<< --mod_indent << '}' << endl;

			attr.getType()->writeCPPSkelMarshalCode(IDL_PARAM_IN,attr.getCPPIdentifier(),
													m_module,mod_indent);
		}
		m_module << --mod_indent << "}" << endl;
	}	
	
}
#endif



/** @brief      Write to the output stream, the skeleton code to unmarshal a CORBA
 *              method and make the call into the corresponding C++ servant.
 *
 *  @param      iface   The orbitcpp interface object which this method belongs to
 *  @param      of      Document Me
 *  @param      node    The IDL tree node which contains info about this method.
 *
 *  This routine contains considerable complexity to account for the different 
 *  memory management schemes employed by CORBA.
 *  The routine is responsible for writing the implementation of a skeleton routine
 *  starting from the method prototype, including the code to unmarshal the C 
 *  structures passed through ORBit to the skeleton. Then the routine must write code
 *  which makes a call to the appropriate C++ servant method.
 *
 *  Because of the memory management issues associated with fixed and variable length
 *  CORBA types, there are two distinct ways of passing a parameter to the servant 
 *  method.
 *
 *  For in and inout types, the type passed through the C skel can be passed directly 
 *  to the servant. For out types, there is typically a need to construct a proxy 
 *  or manager object, as denote by the _out_type of the CORBA type.
 *  The exception to this is arrays of fixed length objects. In this case the _out is
 *  simply a typedef to a T_slice * type. Because the client is responsible for
 *  allocation of the object this gives all the required access to modify individual 
 *  elements inside the servant.
 *  Because variable lenth arrays require a manager object they fall under the general
 *  case.
 *
 *  In terms of the code which is written, T_out object must be constructed from the C
 *  argument passed in. However, for arrays of fixed length objects, the _out is a typedef
 *  to T_slice * so a pointer assignment is required to make the code syntactically 
 *  correct C++.
 *
 *  To determine if the parameter passed is an Array of fixed length objects requires
 *  first dereferencing any aliasing done through typedefs. Then there are simple methods
 *  to determine if the code is an array (dynamic_cast to IDLArray) and contains fixed 
 *  length types (isVariableLength() ).
 */
void 
IDLPassSkels::doOperationSkel (IDLInterface &iface,
			       IDLInterface &of,
			       IDL_tree      node)
{
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	// print header
	m_module << mod_indent << op.skel_ret_get ()
		 << iface.get_cpp_poa_typename () << "::"
		 << "_skel_" << op.get_c_identifier ()
		 << " (" << op.skel_arglist_get () << ")" << endl
		 << mod_indent++ << "{" << endl;

	// inherited => call up base "passthru"
	if (&iface != &of) {

		m_module << mod_indent << of.get_cpp_poa_typename () << "::_orbitcpp_Servant _fake;" << endl;
		m_module << mod_indent << "_fake.m_cppimpl = "
			 << "((_orbitcpp_Servant*)_servant)->m_cppimpl; // causes implicit cast" << endl;
		m_module << mod_indent << "return "
			 << of.get_cpp_poa_typename () << "::"
			 << "_skel_" << op.get_cpp_identifier ()
			 << " (&_fake, ";

		for (IDLOperation::ParameterList::const_iterator i = op.m_parameterinfo.begin ();
		     i != op.m_parameterinfo.end (); i++)
		{
			m_module << i->id << ", ";
		}

		m_module << "_ev);" << endl;

	}
	else {
		// Prepare parameters and return value container
		op.skel_do_pre (m_module, mod_indent);

		// Do the call
		op.skel_do_call (m_module, mod_indent);
		
		// De-init parameters and return value container,
		// FIXME: handle return value
		op.skel_do_post (m_module, mod_indent);
	}

	// End of skel implementation
	m_module << --mod_indent << "}" << endl << endl;

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}


#if 0 //!!!
void 
IDLPassSkels::doAttributePrototype(IDLInterface &iface,IDL_tree node) {
	IDLAttribute &attr = (IDLAttribute &) *iface.getItem(node);
	// get
	string ret_typespec,ret_typedcl;
	attr.getType()->getCPPStubReturnDeclarator(attr.getCPPIdentifier(),ret_typespec,ret_typedcl);

	m_header
	<< indent << "virtual " << ret_typespec << ' ' << ret_typedcl << "()" << endl;
	indent++;
	m_header
	<< indent << "throw ("IDL_CORBA_NS "::SystemException) = 0;" << endl;
	indent--;
	
	// set
	if(!attr.isReadOnly()){
		string type,dcl;
		attr.getType()->getCPPStubDeclarator(IDL_PARAM_IN,"val",type,dcl);
		m_header <<
		indent << "virtual void " << attr.getCPPIdentifier() << '(' << type << ' ' << dcl 
			   << ")" << endl;
		indent++;
		m_header
			<< indent << "throw ("IDL_CORBA_NS "::SystemException) = 0;" << endl;
		indent--;	
	}	
}
#endif



void 
IDLPassSkels::doOperationPrototype (IDLInterface &iface,
				    IDL_tree       node) {
	IDLOperation &op = (IDLOperation &) *iface.getItem(node);

	m_header << indent << "virtual " << op.stub_decl_proto ();

	m_header << ++indent << "throw (" IDL_CORBA_NS "::SystemException";

	for (IDLOperation::ExceptionList::const_iterator i = op.m_raises.begin ();
	     i != op.m_raises.end (); i++)
	{
		m_header << ", " << (*i)->get_cpp_typename ();
	}

	m_header << ") = 0;" << endl;
	indent--;

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}



#if 0 //!!!
void 
IDLPassSkels::doAttributeTie(IDLInterface &iface,IDL_tree node) {
}
#endif



void 
IDLPassSkels::doOperationTie(IDLInterface &iface,IDL_tree node) {
}




// interface-scope methods ----------------------------------------------------
void 
IDLPassSkels::doInterfaceAppServant (IDLInterface &iface)
{
	m_header << indent << "struct _orbitcpp_Servant" << endl
		 << indent++ << "{" << endl;

	string c_POA = "POA_" + iface.get_c_typename ();

	m_header << indent << "//\"Inherit\" from " << c_POA << ", which is a ServantBase-like struct." << endl
		 << indent << IDL_IMPL_C_NS_NOTUSED << c_POA << " m_cservant;" << endl
		 << endl;

	m_header << indent << "//C++-specific stuff:" << endl
		 << indent << "PortableServer::Servant m_cppservant;" << endl
		 << indent << iface.get_cpp_typename () << " *m_cppimpl;  // fully downcasted version of m_cppservant" << endl;
	m_header << --indent << "} m_target;" << endl << endl;
}




void 
IDLPassSkels::doInterfaceEPVs (IDLInterface &iface)
{
	// base_epv decl
	m_header << indent << "static "
		 << "::PortableServer_ServantBase__epv _base_epv;" << endl;

	// base_epv def
	m_module << mod_indent++ <<  "::"
		 << "PortableServer_ServantBase__epv "
		 << iface.get_cpp_poa_typename () << "::_base_epv = {" << endl;

	m_module << mod_indent << "NULL, // _private" << endl
		 << mod_indent << iface.get_cpp_poa_typename () << "::_orbitcpp_fini," << endl
		 << mod_indent << "NULL  // _default_POA" << endl;

	m_module << --mod_indent << "};" << endl << endl;

	// epv handling
	for (IDLInterface::BaseList::const_iterator i = iface.m_allbases.begin ();
	     i != iface.m_allbases.end (); i++)
	{
		declareEPV (iface, **i);
		defineEPV (iface, **i);
	}
	declareEPV (iface, iface);
	defineEPV (iface, iface);

	// vepv decl
	m_header << indent << "static " IDL_IMPL_C_NS_NOTUSED
		 << iface.get_c_poa_vepv () << " _vepv;" << endl;

	// vepv def
	m_module << mod_indent++ << IDL_IMPL_C_NS_NOTUSED
		 << iface.get_c_poa_vepv() << ' '
		 << iface.get_cpp_poa_typename () << "::_vepv = {" << endl;

	m_module << mod_indent << '&' << iface.get_cpp_poa_typename ()
		 << "::_base_epv," << endl;

	for (IDLInterface::BaseList::const_iterator i = iface.m_allbases.begin ();
	     i != iface.m_allbases.end (); i++)
	{
		m_module << mod_indent << "&_"
			 << (*i)->get_c_typename () << "_epv," << endl;
	}

	m_module << mod_indent << "&_"
		 << iface.get_c_typename () << "_epv" << endl;

	m_module << --mod_indent << "};" << endl << endl;
}




void 
IDLPassSkels::declareEPV (IDLInterface &iface,
			  IDLInterface &of)
{
	m_header << indent << "static " IDL_IMPL_C_NS_NOTUSED
		 << of.get_c_poa_epv () << " _"
		 << of.get_c_typename () << "_epv;" << endl;
}




void 
IDLPassSkels::defineEPV (IDLInterface &iface,
			 IDLInterface &of)
{
	m_module << mod_indent++ << IDL_IMPL_C_NS_NOTUSED
		 << of.get_c_poa_epv () << ' '
		 << iface.get_cpp_poa_typename () << "::"
		 << "_" << of.get_c_typename () << "_epv = {" << endl;
	m_module << mod_indent << "0, // _private" << endl;

	IDL_tree body_list = IDL_INTERFACE(of.getNode()).body;
	while (body_list) {
		IDLElement *item;
		switch (IDL_NODE_TYPE(IDL_LIST(body_list).data)) {
#if 0 //!!!
		case IDLN_ATTR_DCL:
			item = of.getItem(IDL_LIST(body_list).data);
			m_module
			<< mod_indent << iface.getQualifiedCPP_POA()
			<< "::_skel__get_" << item->getCPPIdentifier() << ',' << endl;
			{
				IDLAttribute *attr = dynamic_cast<IDLAttribute*>(item);
				if(!attr->isReadOnly()){
					m_module
					<< mod_indent << iface.getQualifiedCPP_POA()
					<< "::_skel__set_" << item->getCPPIdentifier() << ',' << endl;
				}
			}
			break;
#endif
		case IDLN_OP_DCL:
			item = of.getItem(IDL_LIST(body_list).data);
			m_module << mod_indent << iface.get_cpp_poa_typename ()
				 << "::_skel_" << item->get_cpp_identifier () << ',' << endl;
			break;
		default:
			break;
		}
		body_list = IDL_LIST(body_list).next;
	}

	m_module << --mod_indent << "};" << endl << endl;
}




void 
IDLPassSkels::doInterfaceFinalizer (IDLInterface &iface)
{
	m_header << indent << "static void _orbitcpp_fini("
		 <<  "::PortableServer_Servant servant, "
		 <<  "::CORBA_Environment *ev);" << endl;

	m_module << mod_indent << "void " << iface.get_cpp_poa_typename ()
		 << "::_orbitcpp_fini ("
		 <<  "::PortableServer_Servant servant, "
		 <<  "::CORBA_Environment *ev)" << endl
		 << mod_indent++ << "{" << endl;

	m_module << mod_indent << "//Call C __fini():" << endl
		 << mod_indent << IDL_IMPL_C_NS_NOTUSED
		 << iface.get_c_poa_typename () << "__fini (servant, ev);" << endl << endl;
	
	m_module << mod_indent << "//Do C++-specific stuff:" << endl
		 << mod_indent << "_orbitcpp_Servant* pCppServant = reinterpret_cast<_orbitcpp_Servant*>(servant);" << endl
		 << mod_indent << iface.get_cpp_poa_typename () << "* self = pCppServant->m_cppimpl;" << endl
		 << mod_indent << "self->_remove_ref();" << endl;

	m_module << --mod_indent << '}'<< endl << endl;
}




void 
IDLPassSkels::doInterface (IDLInterface &iface)
{
	string ns_begin,ns_end;
	iface.get_cpp_poa_namespace (ns_begin,ns_end);
	
	if (ns_begin.size()) {
		m_header
		<< indent << ns_begin << endl << endl;
	}

	doInterfaceDerive(iface);
	doInterfaceDelegate(iface);

	if (ns_begin.size())
		m_header << indent << ns_end << endl << endl;		
}




void 
IDLPassSkels::doInterfaceDerive(IDLInterface &iface) {
	// class header
	m_header << indent << "class " << iface.get_cpp_poa_identifier () << ": "
		 << "public " << iface.get_cpp_typename ();

	if (iface.m_bases.size ())
	{
		for (IDLInterface::BaseList::const_iterator i = iface.m_bases.begin ();
		     i != iface.m_bases.end (); i++)
		{
			m_header << ", public virtual " << (*i)->get_cpp_poa_identifier ();
		}
	} else {
		m_header << ", public virtal "
			 << IDL_POA_NS "::ServantBase" << endl;
	}
	
	m_header << indent++ << "{" << endl;

	// C interface
	m_header << indent << "// C interface" << endl;
	m_header << --indent++ << "public:" << endl;
	doInterfaceAppServant(iface);
	
	m_header << --indent++ << "protected:" << endl;
	doInterfaceEPVs (iface);
	doInterfaceFinalizer (iface);

	// C methods
	for (IDLInterface::BaseList::const_iterator i = iface.m_allbases.begin ();
	     i != iface.m_allbases.end (); i++)
	{
		doInterfaceUpCall (iface, **i);
	}
	doInterfaceUpCall(iface,iface);

	// C++ interface
	m_header << indent << "// C++ interface" << endl
		 << --indent++ << "public:" << endl;

	// constructor
	m_header << indent << iface.get_cpp_poa_identifier () << " ();" << endl;

	m_module << mod_indent << iface.get_cpp_poa_typename ()  << "::"
		 << iface.get_cpp_poa_identifier () << " ()" << endl
		 << mod_indent++ << "{" << endl;
	
	m_module << mod_indent << "//C Servant:" << endl
		 << mod_indent << "m_target.m_cservant._private = NULL;" << endl
		 << mod_indent << "m_target.m_cservant.vepv = &_vepv;" << endl
		 << endl;

	m_module << mod_indent << "//C++ Servant:" << endl
		 << mod_indent << "m_target.m_cppservant = this;"
		 << " // does an appropriate upcast thunk (Multiple Inheritance)" << endl
		 << mod_indent << "m_target.m_cppimpl = this;" << endl
		 << endl;
	
	m_module << mod_indent << "//Call __init(), passing our \"derived\" C Servant:" << endl
		 << mod_indent << IDL_IMPL_NS "::CEnvironment ev;" << endl
		 << mod_indent << IDL_IMPL_C_NS_NOTUSED << iface.get_c_poa_typename ()
		 << "__init (&m_target, ev._orbitcpp_get_c_object ());" << endl
		 << mod_indent << "ev.propagate_sysex ();" << endl;

	m_module << --mod_indent << '}' << endl << endl;

	// destructor
	m_header << indent << "virtual ~" << iface.get_cpp_poa_identifier () << "()" << endl
		 << indent << "{" << endl
		 << indent << "}" << endl << endl;

	// C++ methods
	m_header << indent <<  "::PortableServer_Servant *_orbitcpp_get_c_servant ()" << endl
		 << indent++ << "{" << endl;
	m_header << indent << "return reinterpret_cast<" 
		 << "::PortableServer_Servant *>"
		 << "(&m_target);" << endl;
	m_header << --indent << '}' << endl << endl;

	// _this() method declaration:
	m_header << indent << iface.get_cpp_typename_ptr () << " _this();" << endl << endl;
	
	// _this() method implementation:
	string stub_name = iface.get_cpp_stub_typename ();

	m_module << mod_indent << iface.get_cpp_typename_ptr () << " "
		 << iface.get_cpp_poa_typename () << "::" << "_this()" << endl
		 << mod_indent++ << "{" << endl;

	m_module << mod_indent << "PortableServer::POA_var rootPOA = _default_POA ();" << endl
		 << mod_indent << "PortableServer::ObjectId_var oid = rootPOA->activate_object (this);" << endl
		 << mod_indent << "CORBA::Object_ptr object = rootPOA->id_to_reference (oid);" << endl;
	m_module << mod_indent << iface.get_cpp_typename_ptr () << "* pDerived = "
		 << "new " << stub_name << " (object->_orbitcpp_get_c_object ());" << endl;
	m_module << mod_indent << "CORBA::release (object);" << endl
		 << mod_indent << "object = 0;" << endl;
	m_module << mod_indent << "return pDerived;" << endl;
	m_module << --mod_indent << "}" << endl << endl;

	
	doInterfacePrototypes(iface);

	// end of class
	m_header << --indent << "};" << endl << endl;
}




void 
IDLPassSkels::doInterfaceUpCall(IDLInterface &iface,IDLInterface &of) {
	IDL_tree body_list = IDL_INTERFACE(of.getNode()).body;
	while (body_list) {
		switch (IDL_NODE_TYPE(IDL_LIST(body_list).data)) {
#if 0 //!!!
		case IDLN_ATTR_DCL:
			doAttributeSkelPrototype(iface,of,IDL_LIST(body_list).data);
			doAttributeSkel(iface,of,IDL_LIST(body_list).data);
			break;
#endif
		case IDLN_OP_DCL:
			doOperationSkelPrototype(iface,of,IDL_LIST(body_list).data);
			doOperationSkel(iface,of,IDL_LIST(body_list).data);
			break;
		default:
			break;
		}
		body_list = IDL_LIST(body_list).next;
	}
}




void 
IDLPassSkels::doInterfacePrototypes(IDLInterface &iface) {
	IDL_tree body_list = IDL_INTERFACE(iface.getNode()).body;
	while (body_list) {
		switch (IDL_NODE_TYPE(IDL_LIST(body_list).data)) {
#if 0 //!!!
		case IDLN_ATTR_DCL:
			doAttributePrototype(iface,IDL_LIST(body_list).data);
			break;
#endif
		case IDLN_OP_DCL:
			doOperationPrototype(iface,IDL_LIST(body_list).data);
			break;
		default:
			break;
		}
		body_list = IDL_LIST(body_list).next;
	}
}




void 
IDLPassSkels::doInterfaceDelegate(IDLInterface &iface) {
/*
	m_header
	<< indent << "class " << iface.getCPP_tie() << " : public "
	<< iface.getCPP_POA() << " {" << endl;
	indent++;

	m_header
	<< --indent << "};" << endl;
*/
}
