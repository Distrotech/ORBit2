/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 1998-2000 Ronald Garcia
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
 *  Author: Ronald Garcia <rgarcia4@darwin.helios.nd.edu>
 *
 *  Description: Portable Object Adaptor Header file, with portions
 *  taken from Micos poa.h
 */




#ifndef __ORBITCPP_POA_HH
#define __ORBITCPP_POA_HH




#include <orb/orbitcpp_smartptr.hh>
#include <orb/orbitcpp_orb.hh>
#include <orb/orbitcpp_object.hh>
#include <orb/orbitcpp_exception.hh>
#include <orb/orbitcpp_sequence.hh>




namespace PortableServer {
	ORBITCPP_DECLARE_SIMPLE_SEQUENCE(CORBA::Octet,
		CORBA_sequence_octet__alloc,
		CORBA_octet_allocbuf,
		PortableServer_ObjectId,
		ObjectId)

	
	class ServantBase;
	typedef ServantBase* Servant;
	
	class ServantBase  {
	public:
		// virtual CORBA::Boolean _is_a(const char *logical_type_id) throw(CORBA::SystemException);

		virtual ~ServantBase();
		virtual POA_ptr _default_POA();

		// The default implementation of these are empty
		virtual void _add_ref();
		virtual void _remove_ref();
		
		// ORBit-C++ extension
		virtual PortableServer_Servant _orbitcpp_get_c_servant() = 0;

		
	protected:
		ServantBase() {
		}
	
		ServantBase(const ServantBase&);
		ServantBase& operator=(const ServantBase&);
	public: // for now
		// NOTE! This needs to be defined somewhere
	
		static PortableServer_ServantBase__epv epv;
	};

	class RefCountServantBase : public virtual ServantBase
	{
	public:
		virtual ~RefCountServantBase();

		virtual void _add_ref();
		virtual void _remove_ref();

	protected:
		RefCountServantBase() : m_ref_count(1) {}
		RefCountServantBase(const RefCountServantBase&) : m_ref_count(1) {}

		RefCountServantBase& operator=(const RefCountServantBase&);

	private:
		// XXX need to make this thread safe
		CORBA::ULong m_ref_count;
	};
	


	class ServantBase_var
	{
	public:
		ServantBase_var() : _ptr(0) {}
		
		ServantBase_var(ServantBase* p) : _ptr(p) {}
		
		ServantBase_var(const ServantBase_var& b) : _ptr(b._ptr)
		{
			if (_ptr != 0) _ptr->_add_ref();
		}

		~ServantBase_var() {
			if (_ptr != 0) _ptr->_remove_ref();
		}

		ServantBase_var& operator=(ServantBase* p) {
			if (_ptr != 0) _ptr->_remove_ref();
			_ptr = p;
			return *this;
		}

		ServantBase_var&
		operator=(const ServantBase_var& b){
			if (_ptr != b._ptr) {
				if (_ptr != 0) _ptr->_remove_ref();
				if ((_ptr = b._ptr) != 0)
					_ptr->_add_ref();
			}
			return *this;
		}

		ServantBase* operator->() const {
			return _ptr;
		}

		ServantBase* in() const {
			return _ptr;
		}
		ServantBase*& inout() {
			return _ptr;
		}
		ServantBase*& out() {
			if (_ptr != 0) _ptr->_remove_ref();
			_ptr = 0;
			return _ptr;
		}

		ServantBase* _retn(){
			ServantBase* retval = _ptr;
			_ptr = 0;
			return retval;
		}

	private:
		ServantBase* _ptr;
	};

	
	class POA : public CORBA::Object {
		friend void release(POA_ptr poa);
		friend CORBA::Object_ptr
		CORBA::ORB::resolve_initial_references(const char* str);
	
	public:
		static void destroy( bool etherealize_objects,
							 bool wait_for_completion );
		POAManager_ptr the_POAManager();
	
		ObjectId *activate_object (Servant srv);
		void activate_object_with_id(ObjectId const &oid, Servant srv );
		void deactivate_object(ObjectId const &oid);
	
		ObjectId *servant_to_id(Servant srv);
		CORBA::Object_ptr servant_to_reference(Servant srv);
		PortableServer::Servant reference_to_servant(CORBA::Object_ptr obj);
		ObjectId *reference_to_id(CORBA::Object_ptr obj);
		PortableServer::Servant id_to_servant(ObjectId const &oid);
		CORBA::Object_ptr id_to_reference(ObjectId const &oid);
	
		static POA_ptr _duplicate(POA_ptr obj);
		static POA_ptr _narrow(CORBA::Object_ptr obj);
		static POA_ptr _nil();
	
	~POA() {}
	
	protected:
	
	private:
		//PortableServer_POA_type m_target;
		// Copy, assignment, and default constructor not wanted
	
		POA();
		POA(const POA &);
		void operator=(const POA &);
	
		PortableServer_POA get_c_poa() {
			return reinterpret_cast<PortableServer_POA>(&m_target);
		}
	};
	
	
	class POAManager : public CORBA::Object {
		friend class POA;
	public:
		static POAManager_ptr _duplicate( POAManager_ptr obj );
		static POAManager_ptr _narrow( CORBA::Object_ptr obj );
		static POAManager_ptr _nil();
		void activate();
		void hold_requests(CORBA::Boolean wait_for_completion);
		void discard_requests(CORBA::Boolean wait_for_completion);
		void deactivate(CORBA::Boolean etherealize_objects,
						CORBA::Boolean wait_for_completion);
	
	protected:
		POAManager(PortableServer_POAManager o);
		~POAManager();
	
	private:
		PortableServer_POAManager get_c_poamgr() {
			return reinterpret_cast<PortableServer_POAManager>(&m_target);
		}
	};
	
}




namespace _orbitcpp {
	template<class T_ptr>
	class ServantMethods : public virtual PortableServer::ServantBase {
	public:
		
		T_ptr _this() {
			PortableServer::POA_var rootPOA = _default_POA();
			PortableServer::ObjectId_var oid = rootPOA->activate_object(this);
			CORBA::Object_ptr object = rootPOA->id_to_reference(oid);
			return reinterpret_cast<T_ptr>(object);
		}
	};
}




namespace CORBA {
// the mandatory release function
void release(PortableServer::POA_ptr poa);
Boolean is_nil(PortableServer::POA_ptr poa);
}




#endif // __ORBITCPP_POA_HH
