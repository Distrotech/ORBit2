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


#ifndef ORBITCPP_TYPES_IDLTYPECODE
#define ORBITCPP_TYPES_IDLTYPECODE

#include "IDLInterface.hh"

class IDLTypeCode : public IDLInterface
{
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


};

#endif //ORBITCPP_TYPES_IDLTYPECODE
