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


#ifndef ORBITCPP_TYPES_IDLOBJECT
#define ORBITCPP_TYPES_IDLOBJECT

#include "IDLInterface.hh"

class IDLObject : public IDLInterface
{
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

	

};

#endif //ORBITCPP_TYPES_IDLOBJECT

