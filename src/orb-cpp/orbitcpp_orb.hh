/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 1998-2000 Phil Dawes
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




#ifndef __ORBITCPP_ORB_HH
#define __ORBITCPP_ORB_HH




#include "orbitcpp_object.hh"
#include <string>

namespace CORBA {
	CORBA::ORB_ptr ORB_init(int& argc, char** argv,
		const char* orb_identifier="orbit-local-orb");
	
	// the mandatory release function
	void release(ORB_ptr orb);
	
	class ORB {
		friend void release(ORB_ptr orb);
		// allow ORB_init to create this object
		friend CORBA::ORB_ptr ORB_init(int& argc, char** argv,
									   const char* orb_identifier);

	public:
		explicit ORB(CORBA_ORB cobject);

		void operator delete(void* cpp_orb) {
			Object* pObject = static_cast<Object*>(cpp_orb);
			_orbitcpp::release_guarded(pObject->_orbitcpp_cobj());
		}
	
	
		static ORB_ptr _duplicate(ORB_ptr o);
		static ORB_ptr _narrow(Object_ptr obj);
		static ORB_ptr _nil() {
			return NULL;
		}

		Object_ptr string_to_object(const char* str);
		Object_ptr string_to_object(const std::string &str);
		char* object_to_string(Object_ptr obj);
	
// *** FIXME implement all this stuff
/*		Status create_list(Long, NVList_out); 
		Status create_operation_list(OperationDef_ptr,NVList_out); 
		
		Status create_named_value(NamedValue_out); 
		Status create_exception_list(ExceptionList_out); 
		Status create_context_list(ContextList_out);
		
		Status get_default_context(Context_out); 
		Status create_environment(Environment_out);
		
		Status send_multiple_requests_oneway( const RequestSeq& ); 
		Status send_multiple_requests_deferred( const RequestSeq& ); 
		
		Boolean poll_next_response();
		Status get_next_response(Request_out); 
		
		typedef char* ObjectId; 
		class ObjectIdList { }; 
		class InvalidName { }; 
		
		ObjectIdList *list_initial_services(); */
		Object_ptr resolve_initial_references(const char *identifier);
  
		Boolean work_pending();
		void perform_work();
		void shutdown(Boolean wait_for_completion);
		void run();

//      Boolean get_service_information(ServiceType svc_type,ServiceInformation_out svc_info);
	
	private:
		void operator=(const ORB&);

		CORBA_ORB m_target;	// the C ORB we are wrapping
	};
}

#endif //__ORBITCPP_ORB_HH
