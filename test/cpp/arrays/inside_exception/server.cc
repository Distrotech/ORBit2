/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "test-impl.hh"
#include <iostream>
#include <fstream>

int main (int argc, char* argv[])
{
	// Initialize the CORBA orb
	CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);
	
	// Get the root POA
	CORBA::Object_var pfobj = orb->resolve_initial_references("RootPOA");
	
	PortableServer::POA_var rootPOA = PortableServer::POA::_narrow(pfobj);
	
	// Activate the root POA's manager
	PortableServer::POAManager_var mgr = rootPOA->the_POAManager();
	
	mgr->activate();
	
	// Create a Servant and explicitly create a CORBA object
	IMaster_impl master_impl;
	CORBA::Object_var master_ptr = master_impl._this();
	
	// Here we get the IOR for the Hello server object.
	// Our "client" will use the IOR to find the object to connect to
	CORBA::String_var ior = orb->object_to_string (master_ptr);
	
	// print out the IORs to files
	std::ofstream ior_file ("test.ior");
	ior_file << ior << std::endl;
	
	// run the server event loop
	orb->run();
}
