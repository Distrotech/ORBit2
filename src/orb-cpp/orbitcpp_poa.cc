/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

/*
 *  ORBit-C++: C++ bindings for ORBit.
 *
 *  Copyright (C) 1998 Ronald Garcia
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
 *  Description: Portable Object Adaptor Implementation
 */




#include "orbitcpp_poa.hh"




using namespace _orbitcpp;




PortableServer_ServantBase__epv
PortableServer::ServantBase::epv = {
    NULL,
    NULL,
    NULL
};




// PortableServer::ServantBase ------------------------------------------------

PortableServer::ServantBase::~ServantBase() {
}

PortableServer::POA_ptr
PortableServer::ServantBase::_default_POA() {
	// orbit caches the orb, so there's no point in us doing the same
	int argc=0;	char* argv=NULL;
	CORBA::ORB_ptr orb = CORBA::ORB_init(argc,&argv);
	CORBA::Object_var pfobj = orb->resolve_initial_references("RootPOA");
	return PortableServer::POA::_narrow(pfobj);
}

void
PortableServer::ServantBase::_add_ref(){
}

void
PortableServer::ServantBase::_remove_ref(){
}

// PortableServer::RefCountServantBase ------------------------------------------------

PortableServer::RefCountServantBase::~RefCountServantBase(){
}

void
PortableServer::RefCountServantBase::_add_ref(){
	// XXX need to make this thread safe
	m_ref_count++;
}

void
PortableServer::RefCountServantBase::_remove_ref(){
	// XXX need to make this thread safe
	if(--m_ref_count == 0)
		delete this;
}


// PortableServer::POA ------------------------------------------------


PortableServer::POA::POA(PortableServer_POA cobject)
: CORBA::Object((CORBA_Object)cobject)
{
}

PortableServer::POAManager_ptr
PortableServer::POA::the_POAManager() {
	CEnvironment ev;
	PortableServer_POAManager mgr = PortableServer_POA__get_the_POAManager(_orbitcpp_cobj(), ev._orbitcpp_cobj());
	ev.propagate_sysex();
	return new PortableServer::POAManager(mgr);
}




PortableServer::POA_ptr
PortableServer::POA::_duplicate( PortableServer::POA_ptr _obj ) {
	CEnvironment ev;
	PortableServer_POA newcpoa =
		(PortableServer_POA)
	    CORBA_Object_duplicate((CORBA_Object)_obj->_orbitcpp_cobj(), ev._orbitcpp_cobj());
	ev.propagate_sysex();

	return new PortableServer::POA(newcpoa);
}




PortableServer::POA_ptr
PortableServer::POA::_narrow( CORBA::Object_ptr obj ) {
	return _duplicate(static_cast<PortableServer::POA_ptr>(obj));
}




PortableServer::POA_ptr
PortableServer::POA::_nil() {
	return NULL;
}




PortableServer::ObjectId *
PortableServer::POA::activate_object (PortableServer::Servant srv) {
	CEnvironment ev;
	srv->_add_ref();
	// Get the C servant structure out of the object
	PortableServer_Servant c_servant = srv->_orbitcpp_get_c_servant();
	PortableServer_ObjectId* coid =
	    PortableServer_POA_activate_object(_orbitcpp_cobj(),
														 c_servant, ev._orbitcpp_cobj());
	ev.propagate_sysex();
	return reinterpret_cast<PortableServer::ObjectId *>(coid);
}




void
PortableServer::POA::activate_object_with_id(PortableServer::ObjectId const &oid,
PortableServer::Servant srv ) {
	error("NYI");
}


//#define ORBIT_05X    // need to put this in an autoconf file

void
PortableServer::POA::deactivate_object(PortableServer::ObjectId const &oid) {
	CEnvironment ev;
	PortableServer_ObjectId *coid =
		reinterpret_cast<PortableServer_ObjectId *>(const_cast<PortableServer::ObjectId*>(&oid));

#ifdef ORBIT_05X
	PortableServer_ServantBase	*servant = (PortableServer_ServantBase*) PortableServer_POA_id_to_servant(_orbitcpp_cobj(), coid, ev._orbitcpp_cobj());
	
	//PortableServer_Servant_var servant = id_to_servant(oid);
	//PortableServer_ServantBase 
#endif

	PortableServer_POA_deactivate_object(_orbitcpp_cobj(), coid, ev._orbitcpp_cobj());

#ifdef ORBIT_05X
	PortableServer_ServantBase__epv *epv = servant->vepv[0];
	/* In theory, the finalize fnc should always be non-NULL;
	 * however, for backward compat. and general extended
	 * applications we dont insist on it.
	 */
	if ( epv && epv->finalize ) {
		(*(epv->finalize))(servant, ev._orbitcpp_cobj());
	}
#endif

}




PortableServer::ObjectId *
PortableServer::POA::servant_to_id(PortableServer::Servant srv) {
	CEnvironment ev;
	PortableServer_Servant c_servant = srv->_orbitcpp_get_c_servant();
	PortableServer_ObjectId* coid =
	    PortableServer_POA_servant_to_id(_orbitcpp_cobj(), c_servant, ev._orbitcpp_cobj());
	ev.propagate_sysex();
	return reinterpret_cast<PortableServer::ObjectId *>(coid);
}




CORBA::Object_ptr
PortableServer::POA::servant_to_reference(PortableServer::Servant srv) {
	CEnvironment ev;
	PortableServer_Servant c_servant = srv->_orbitcpp_get_c_servant();
	CORBA_Object o =
	    PortableServer_POA_servant_to_reference(_orbitcpp_cobj(), c_servant, ev._orbitcpp_cobj());
	ev.propagate_sysex();
	
	return new CORBA::Object(o);

}



// HACK ALERT!!  This is a binary compatible to aid extraction of the
// cppservant from C servants. This *must* be kept in sync with what
// the compiler produces. (if anybody has a better suggestion, please
// mail philipd@users.sourceforge.net)
struct _orbitcpp_Servant {
   struct {
      void *_private;
      void *vepv;
   } cobj;
	::PortableServer::Servant m_cppservant;
	void *m_cppImpl;
} m_target;


PortableServer::Servant
PortableServer::POA::reference_to_servant(CORBA::Object_ptr obj) {
	CEnvironment ev;
	CORBA_Object cobj = obj->_orbitcpp_cobj();
	PortableServer_Servant c_servant = PortableServer_POA_reference_to_servant(_orbitcpp_cobj(),cobj, ev._orbitcpp_cobj());
	PortableServer::Servant srv = ((_orbitcpp_Servant *)c_servant)->m_cppservant;
	return srv;
}



PortableServer::ObjectId *
PortableServer::POA::reference_to_id(CORBA::Object_ptr obj) {
	CEnvironment ev;
	PortableServer_ObjectId* coid =
	    PortableServer_POA_reference_to_id(_orbitcpp_cobj(), obj->_orbitcpp_cobj(), ev._orbitcpp_cobj());
	ev.propagate_sysex();
	return reinterpret_cast<PortableServer::ObjectId *>(coid);
}




PortableServer::Servant
PortableServer::POA::id_to_servant(PortableServer::ObjectId const &oid) {
	CEnvironment ev;
	PortableServer_ObjectId* c_oid = oid._orbitcpp_get_c_sequence_ptr();
	PortableServer_Servant c_servant = PortableServer_POA_id_to_servant(_orbitcpp_cobj(),c_oid, ev._orbitcpp_cobj());
	PortableServer::Servant srv = ((_orbitcpp_Servant *)c_servant)->m_cppservant;
	return srv;
}




CORBA::Object_ptr
PortableServer::POA::id_to_reference(ObjectId const &oid) {
	CEnvironment ev;
	PortableServer_ObjectId* c_oid = oid._orbitcpp_get_c_sequence_ptr();
	CORBA_Object o =
	    PortableServer_POA_id_to_reference(_orbitcpp_cobj(), c_oid, ev._orbitcpp_cobj());
	ev.propagate_sysex();
	
	return new CORBA::Object(o);
}




void PortableServer::POA::destroy(bool, bool) {
	error("NYI");
}




// PortableServer::POAManager -------------------------------------------------

PortableServer::POAManager::POAManager(PortableServer_POAManager cobject)
: CORBA::Object((CORBA_Object)cobject)
{
}

PortableServer::POAManager_ptr
PortableServer::POAManager::_duplicate(PortableServer::POAManager_ptr _obj) {
	POAManager_ptr result = NULL;
	result = _obj;
	return result;
}




PortableServer::POAManager_ptr
PortableServer::POAManager::_nil() {
	return NULL;
}




void
PortableServer::POAManager::activate() {
	CEnvironment ev;
	PortableServer_POAManager_activate(_orbitcpp_cobj(), ev._orbitcpp_cobj());
	ev.propagate_sysex();
}




void
PortableServer::POAManager::hold_requests(CORBA::Boolean wait) {
	CEnvironment ev;
	PortableServer_POAManager_hold_requests(_orbitcpp_cobj(), wait, ev._orbitcpp_cobj());
	ev.propagate_sysex();
}




void
PortableServer::POAManager::discard_requests (CORBA::Boolean wait) {
	CEnvironment ev;
	PortableServer_POAManager_discard_requests(_orbitcpp_cobj(), wait, ev._orbitcpp_cobj());
	ev.propagate_sysex();
}

void
PortableServer::POAManager::deactivate(CORBA::Boolean etherealize,CORBA::Boolean wait) {
	CEnvironment ev;
	PortableServer_POAManager_deactivate(_orbitcpp_cobj(), etherealize, wait, ev._orbitcpp_cobj());
	ev.propagate_sysex();
}




void
CORBA::release(PortableServer::POA_ptr poa) {
	delete poa;
}
