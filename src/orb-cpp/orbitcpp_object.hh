/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 1998 Phil Dawes
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
 *  Author: Phil Dawes <philipd@users.sourceforge.net>
 *
 */

#ifndef __ORBITCPP_OBJECT_HH
#define __ORBITCPP_OBJECT_HH




#include "orbitcpp_types.hh"
#include "orbitcpp_tools.hh"
#include "orbitcpp_smartptr.hh"




namespace CORBA {

	// RG - ServerlessObj
	// This may possibly serve to separate the Server'ed objects from
	// the CORBA internal ones. We may not need it, though, if ORBit-C
	// conveniently throws exceptions when it's serverless structures
	// are used incorrectly (marshalling, object_to_string, etc).
	// Will do research.
	class ServerlessObj;
	typedef ServerlessObj* ServerlessObj_ptr;
	
	class ServerlessObj {
		friend void release (ServerlessObj_ptr o);
	public:
		static ServerlessObj_ptr _duplicate(ServerlessObj_ptr o);
		static ServerlessObj_ptr _narrow(ServerlessObj_ptr obj);
		static ServerlessObj_ptr _nil();
	};
	
	
	
	class Object
	{
		
	public:
		// begin ORBit-C++ extension
		Object(CORBA_Object cobject, bool take_copy = true);
		CORBA_Object& _orbitcpp_get_c_object();
		Object* _orbitcpp_wrap(CORBA_Object cobject, bool take_copy = true);
		// end ORBit-C++ extension

	protected:
		Object();
		virtual ~Object();

  private:
		void operator=(const Object&);

	  friend void release(Object_ptr o);
	
  public:	
		void operator delete(void* c_objref);
		
		static Object_ptr _duplicate(Object_ptr o);
		static Object_ptr _narrow(Object_ptr o);
		static Object_ptr _nil();

		// *** FIXME: Need to implement this 
		// InterfaceDef_ptr _get_interface(); 
		Boolean _is_a(RepositoryId const repoid);
		Boolean _non_existent(); 
		Boolean _is_equivalent(Object_ptr other_object);
		ULong _hash(::CORBA::ULong maximum); 
	
	protected:
		CORBA_Object m_target;
	};
	
	
	inline void release(Object_ptr o)
	{
		_orbitcpp::release_guarded(o->_orbitcpp_get_c_object());
	}
	
	Boolean is_nil(Object_ptr o);
}




#endif
