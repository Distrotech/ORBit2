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





void 
IDLPassSkels::doOperationSkelPrototype(IDLInterface &iface,IDLInterface &of,IDL_tree node) {
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	string ret_typespec,ret_typedcl;
	op.m_returntype->getCSkelReturnDeclarator("_skel_" + op.getCPPIdentifier(),ret_typespec,ret_typedcl);
	m_header
	<< indent << "static " << ret_typespec << ' ' << ret_typedcl << '('
	<<  "::PortableServer_Servant _servant,"
	<< op.getCOpParameterList();
	if (op.m_parameterinfo.size()) m_header << ",";
	m_header
	<<  "::CORBA_Environment *_ev);" << endl;

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}




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
IDLPassSkels::doOperationSkel(IDLInterface & iface,
							  IDLInterface & of,
							  IDL_tree node) {
	IDLOperation &op = (IDLOperation &) *of.getItem(node);

	// print header
	string ret_typespec,ret_typedcl;
	op.m_returntype->getCSkelReturnDeclarator(
	    iface.getQualifiedCPP_POA() + "::_skel_" + op.getCPPIdentifier(),
	    ret_typespec,ret_typedcl
	);

	m_module
	<< mod_indent << ret_typespec << ' ' << ret_typedcl << '('
	<<  "::PortableServer_Servant _servant,"
	<< op.getCOpParameterList();
	if (op.m_parameterinfo.size()) m_module << ',';
	m_module
	<<  "::CORBA_Environment *_ev) {" << endl;
	mod_indent++;

	// inherited => call up base "passthru"
	if (&iface != &of) {
		m_module
		<< mod_indent << of.getQualifiedCPP_POA() << "::_orbitcpp_Servant _fake;" << endl
		<< mod_indent << "_fake.m_cppimpl = ((_orbitcpp_Servant *)_servant)->m_cppimpl; // causes implicit cast" << endl;
		
		m_module
		<< mod_indent << "return "
		<< of.getQualifiedCPP_POA() << "::_skel_" << op.getCPPIdentifier()
		<< "(&_fake,";
		
		IDLOperation::ParameterList::const_iterator
		firstp = op.m_parameterinfo.begin(),lastp = op.m_parameterinfo.end();
	
		while (firstp != lastp) {
			m_module << firstp->Identifier << ',';
			firstp++;
		}
		
		m_module
		<< "_ev);" << endl;

	}
	else {
		// unmarshal parameters
		IDLOperation::ParameterList::const_iterator firstp = 
			op.m_parameterinfo.begin(),lastp = op.m_parameterinfo.end();
	
		while (firstp != lastp) {
			firstp->Type->writeCPPSkelDemarshalCode(firstp->Direction,firstp->Identifier,m_module,mod_indent);
			firstp++;
		}
	
		op.m_returntype->writeCPPSkelReturnPrepCode(m_module,mod_indent);
		m_module
		<< mod_indent << "bool _results_valid = true;" << endl << endl;
	
		// do the call
		m_module << mod_indent << "try {" << endl; // open try block

		mod_indent++;
		m_module << mod_indent << iface.getQualifiedCPP_POA() << " *_self = ";
		m_module << "((_orbitcpp_Servant *)_servant)->m_cppimpl;" << endl;

		// RJA inserted code

		m_module << endl;

		firstp = op.m_parameterinfo.begin();
		lastp = op.m_parameterinfo.end();

		std::vector<std::string> servantArgs;

		unsigned int wrapperCounter = 0;

		while (firstp != lastp) 
		{
			string typespec;
			string decl;
			firstp->Type->getCPPStubDeclarator( firstp->Direction, 
												firstp->Identifier,
												typespec,
												decl );

			bool makeWrapper = false;

			if ( firstp->Direction == IDL_PARAM_OUT )
			{
				makeWrapper = true;
			}

			// Remove any aliasing due to typedefs
			IDLType const & realType = (firstp->Type)->getResolvedType();
			
			// At this point we can guarantee that realType is not a typedef
			// so we can get information about the real type represented in the arg.

			bool pointerAssignment = false; // true when we don't have a real _out type
			
			if ( 0 != dynamic_cast<const IDLArray *>(&realType) ) {
				if ( !( realType.isVariableLength() ) )	{
					pointerAssignment = true;
				}
			}


			if ( makeWrapper && (false == pointerAssignment) ) {
				// In this case define a local var, named <arg> of the type expected 
				// by the servant operation and then pass <arg> as an argument later

				std::ostringstream arg; // stringstream to convert integer type to string
				arg << string("orbitcppSkelWrapper_out_");
				arg << wrapperCounter;

				m_module << mod_indent << typespec << " " << arg.str();
				m_module << "( ";


				m_module << firstp->Type->getCPPSkelParameterTerm( firstp->Direction,
																   firstp->Identifier );

				m_module << " )";
				m_module << ";" << endl; // end of the call

				servantArgs.push_back( arg.str() );
			}
			else {
				// In this case no local var is necessary so just pass the whole 
				// ugly string through to the servant operation.
				// This is necessary when the arg is an array of fixed length type.
				string arg( firstp->Type->getCPPSkelParameterTerm( firstp->Direction,
																   firstp->Identifier ) );

				servantArgs.push_back(arg);
			}

			++firstp;
			++wrapperCounter;
		}


		// some_type _retval = _self->opName( arg0, arg1, .... );
		m_module << mod_indent << op.m_returntype->getCPPSkelReturnAssignment();

		m_module << "_self->" << op.getCPPIdentifier() << '(';
	
		typedef std::vector<std::string>::const_iterator args_iterator;
		args_iterator argsIt = servantArgs.begin();
		args_iterator argsEnd = servantArgs.end();
		

		while (argsIt != argsEnd) {
			m_module << (*argsIt);

			++argsIt;
			if (argsIt != argsEnd) {
				m_module << ", ";
			}
		}

		m_module << ");" << endl; // close parameter list

		m_module << --mod_indent << '}' << endl; // close try block
	
		// handle exceptions
		m_module
		<< mod_indent << "catch ("IDL_CORBA_NS "::Exception &_ex) {" << endl;
		mod_indent++;
		m_module
		<< mod_indent << "_results_valid = false;" << endl
		<< mod_indent << "_ex._orbitcpp_set(_ev);" << endl;
		m_module
		<< --mod_indent << '}' << endl;
	
		m_module
		<< mod_indent << "catch (...) {" << endl;
		m_module
		<< ++mod_indent << IDL_IMPL_NS "::error(\"unknown exception in skeleton\");" << endl;
		m_module
		<< --mod_indent << '}' << endl << endl;
	
		m_module
		<< mod_indent << "if (!_results_valid) " << op.m_returntype->getInvalidReturn() << endl;
	
		// marshal parameters
		firstp = op.m_parameterinfo.begin();
		lastp = op.m_parameterinfo.end();
	
		while (firstp != lastp) {
			firstp->Type->writeCPPSkelMarshalCode(firstp->Direction,firstp->Identifier,m_module,mod_indent);
			firstp++;
		}
	
		op.m_returntype->writeCPPSkelReturnMarshalCode(m_module,mod_indent);
	}
	// end function
	m_module
	<< --mod_indent << '}' << endl << endl;

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}




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




void 
IDLPassSkels::doOperationPrototype(IDLInterface &iface,IDL_tree node) {
	IDLOperation &op = (IDLOperation &) *iface.getItem(node);

	string ret_typespec,ret_typedcl;
	op.m_returntype->getCPPStubReturnDeclarator(op.getCPPIdentifier(),ret_typespec,ret_typedcl);

	m_header
	<< indent << "virtual " << ret_typespec << ' ' << ret_typedcl << '('
	<< op.getCPPOpParameterList() << ')' << endl;

	indent++;
	m_header
	<< indent << "throw ("IDL_CORBA_NS "::SystemException";
	IDLOperation::ExceptionList::const_iterator firstx = op.m_raises.begin(),
	        lastx = op.m_raises.end();

	while (firstx != lastx && *firstx)
		m_header
		<< ',' << (*firstx++)->getQualifiedCPPIdentifier();

	m_header
	<< ") = 0;" << endl;
	indent--;

	if (IDL_OP_DCL(node).context_expr != NULL) ORBITCPP_NYI("contexts");
}




void 
IDLPassSkels::doAttributeTie(IDLInterface &iface,IDL_tree node) {
}




void 
IDLPassSkels::doOperationTie(IDLInterface &iface,IDL_tree node) {
}




// interface-scope methods ----------------------------------------------------
void 
IDLPassSkels::doInterfaceAppServant(IDLInterface &iface) {
	m_header
	<< indent << "struct _orbitcpp_Servant {" << endl;
	indent++;
	m_header
	<< indent << IDL_IMPL_C_NS "::"
	<< iface.getQualifiedC_POA() << " m_cservant;" << endl
	<< indent << "PortableServer::Servant m_cppservant;" << endl
	<< indent << iface.getQualifiedCPP_POA() << " *m_cppimpl;  // fully downcasted version of m_cppservant" << endl;
	m_header
	<< --indent << "} m_target;" << endl;
}




void 
IDLPassSkels::doInterfaceEPVs(IDLInterface &iface) {
	// base_epv decl
	m_header
	<< indent << "static " "::"
	<< "PortableServer_ServantBase__epv _base_epv;" << endl;

	// base_epv def
	m_module
	<< mod_indent <<  "::"
	<< "PortableServer_ServantBase__epv "
	<< iface.getQualifiedCPP_POA() << "::_base_epv = {" << endl;
	mod_indent++;
	m_module
	<< mod_indent << "NULL, // _private" << endl
	<< mod_indent << iface.getQualifiedCPP_POA() << "::_orbitcpp_fini, // _fini" << endl
	<< mod_indent << "NULL  // _default_POA" << endl;
	m_module
	<< --mod_indent << "};" << endl << endl;

	// epv handling
	IDLInterface::BaseList::const_iterator
	first = iface.m_allbases.begin(),last=iface.m_allbases.end();
	while (first != last) {
		declareEPV(iface,**first);
		defineEPV(iface,**first++);
	};
	declareEPV(iface,iface);
	defineEPV(iface,iface);

	// vepv decl
	m_header
	<< indent << "static " IDL_IMPL_C_NS "::"
	<< iface.getQualifiedC_vepv() << " _vepv;" << endl;

	// vepv def
	m_module
	<< mod_indent << IDL_IMPL_C_NS "::"
	<< iface.getQualifiedC_vepv() << ' '
	<< iface.getQualifiedCPP_POA() << "::_vepv = {" << endl;
	mod_indent++;

	m_module
	<< mod_indent << '&' << iface.getQualifiedCPP_POA()
	<< "::_base_epv," << endl;

	first = iface.m_allbases.begin();
	last = iface.m_allbases.end();

	while (first != last) {
		m_module
		<< mod_indent << "&_" << (*first)->getQualifiedCIdentifier() << "_epv," << endl;
		first++;
	}

	m_module
	<< mod_indent << "&_" << iface.getQualifiedCIdentifier() << "_epv" << endl;

	m_module
	<< --mod_indent << "};" << endl << endl;
}




void 
IDLPassSkels::declareEPV(IDLInterface &iface,IDLInterface &of) {
	m_header
	<< indent << "static " IDL_IMPL_C_NS "::"
	<< of.getQualifiedC_epv() << " _"
	<< of.getQualifiedCIdentifier() << "_epv;" << endl;
}




void 
IDLPassSkels::defineEPV(IDLInterface &iface,IDLInterface &of) {
	m_module
	<< mod_indent << IDL_IMPL_C_NS "::"
	<< of.getQualifiedC_epv() << ' '
	<< iface.getQualifiedCPP_POA() << "::"
	<< "_" << of.getQualifiedCIdentifier() << "_epv = {" << endl;
	mod_indent++;
	m_module
	<< mod_indent << "NULL, // _private" << endl;
	
	IDL_tree body_list = IDL_INTERFACE(of.getNode()).body;
	while (body_list) {
		IDLElement *item;
		switch (IDL_NODE_TYPE(IDL_LIST(body_list).data)) {
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
		case IDLN_OP_DCL:
			item = of.getItem(IDL_LIST(body_list).data);
			m_module
			<< mod_indent << iface.getQualifiedCPP_POA()
			<< "::_skel_" << item->getCPPIdentifier() << ',' << endl;
			break;
		default:
			break;
		}
		body_list = IDL_LIST(body_list).next;
	}

	m_module
	<< --mod_indent << "};" << endl << endl;
}




void 
IDLPassSkels::doInterfaceFinalizer(IDLInterface &iface) {
	m_header
	<< indent << "static void _orbitcpp_fini("
	<<  "::PortableServer_Servant servant,"
	<<  "::CORBA_Environment *ev);" << endl;

	m_module
	<< mod_indent << "void " << iface.getQualifiedCPP_POA()
	<< "::_orbitcpp_fini("
	<<  "::PortableServer_Servant servant,"
	<<  "::CORBA_Environment *ev) {" << endl;
	m_module
	<< ++mod_indent << IDL_IMPL_C_NS "::"
	<< iface.getQualifiedC_POA() << "__fini(servant,ev);" << endl;
	m_module
	<< mod_indent << iface.getQualifiedCPP_POA() << " *self = "
	<< "reinterpret_cast<_orbitcpp_Servant *>(servant)->m_cppimpl;" << endl
	<< mod_indent << "self->_remove_ref();" << endl;
	m_module
	<< --mod_indent << '}'<< endl << endl;
}




void 
IDLPassSkels::doInterface(IDLInterface &iface) {
	string ns_begin,ns_end;
	iface.getCPPpoaNamespaceDecl(ns_begin,ns_end);

	if (ns_begin.size()) {
		m_header
		<< indent << ns_begin << endl;
		indent++;
	}

	doInterfaceDerive(iface);
	doInterfaceDelegate(iface);

	if (ns_begin.size())
		m_header << --indent << ns_end << endl << endl;		
}




void 
IDLPassSkels::doInterfaceDerive(IDLInterface &iface) {
	// class header
	m_header
	<< indent << "class " << iface.getCPP_POA() << " : "
	<< "public " << iface.getQualifiedCPPIdentifier();

	if (iface.m_bases.size() > 0) {
		IDLInterface::BaseList::const_iterator
		first = iface.m_bases.begin(), last = iface.m_bases.end();
		
		while (first != last)
			m_header << ", public virtual " << (*first++)->getQualifiedCPP_POA();
		m_header << " {" << endl;
	}
	else
		m_header
		<< ", public virtual "IDL_POA_NS "::ServantBase {" << endl;

	// C interface
	m_header
	<< indent << "// C interface" << endl
	<< indent << "public:" << endl;
	indent++;

	doInterfaceAppServant(iface);
	m_header << endl;
	
	m_header
	<< --indent << "protected:" << endl;
	indent++;
	
	doInterfaceEPVs(iface);
	m_header << endl;
	doInterfaceFinalizer(iface);

	// C methods
	IDLInterface::BaseList::const_iterator
	firstb = iface.m_allbases.begin(),lastb = iface.m_allbases.end();

	while (firstb != lastb)
		doInterfaceUpCall(iface,**firstb++);
	doInterfaceUpCall(iface,iface);

	// C++ interface
	indent--;
	m_header
	<< indent << "// C++ interface" << endl
	<< indent << "public:" << endl;
	indent++;

	// constructor
	m_header
	<< indent << iface.getCPP_POA() << "();" << endl;

	m_module
	<< mod_indent << iface.getQualifiedCPP_POA() 
	<< "::" << iface.getCPP_POA() << "() {" << endl;
	m_module
	<< ++mod_indent << "m_target.m_cservant._private = NULL;" << endl
	<< mod_indent << "m_target.m_cservant.vepv = &_vepv;" << endl
	<< mod_indent << "m_target.m_cppservant = this; // does an appropriate upcast thunk" << endl
	<< mod_indent << "m_target.m_cppimpl = this;" << endl
	<< mod_indent << IDL_IMPL_NS "::CEnvironment ev;" << endl
	<< mod_indent << IDL_IMPL_C_NS "::" << iface.getQualifiedC_POA()
	<< "__init(&m_target.m_cservant,ev);" << endl
	<< mod_indent << "ev.propagate_sysex();" << endl;
	m_module
	<< --mod_indent << '}' << endl << endl;

	// destructor
	m_header
	<< indent << "virtual ~" << iface.getCPP_POA() << "() {"
	<< indent << "}" << endl;

	// C++ methods
	m_header
	<< indent <<  "::PortableServer_Servant *_orbitcpp_get_c_servant() {" << endl;
	m_header
	<< ++indent << "return reinterpret_cast< " 
	<< "::PortableServer_Servant *>(&m_target);" << endl;
	m_header
	<< --indent << '}' << endl << endl;

	// _this() method
	m_header
		<< indent << iface.getQualifiedCPP_ptr() << " _this() {" << endl;
	m_header
	  << ++indent << "PortableServer::POA_var rootPOA = _default_POA();" << endl
	  << indent << "PortableServer::ObjectId_var oid = rootPOA->activate_object(this);" << endl
	  << indent << "CORBA::Object_ptr object = rootPOA->id_to_reference(oid);" << endl
	  << indent << "return reinterpret_cast< "
	  << iface.getQualifiedCPPStub() << " *>(object);" << endl; 
	m_header
	  << --indent << "}" << endl << endl;


	
	doInterfacePrototypes(iface);

	// end of class
	m_header
	<< --indent << "};" << endl;
}




void 
IDLPassSkels::doInterfaceUpCall(IDLInterface &iface,IDLInterface &of) {
	IDL_tree body_list = IDL_INTERFACE(of.getNode()).body;
	while (body_list) {
		switch (IDL_NODE_TYPE(IDL_LIST(body_list).data)) {
		case IDLN_ATTR_DCL:
			doAttributeSkelPrototype(iface,of,IDL_LIST(body_list).data);
			doAttributeSkel(iface,of,IDL_LIST(body_list).data);
			break;
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
		case IDLN_ATTR_DCL:
			doAttributePrototype(iface,IDL_LIST(body_list).data);
			break;
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
