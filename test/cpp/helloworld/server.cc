/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include <iostream>
#include "helloworld-impl.hh"

int main (int argc, char* argv[])
{
	// Initialize the CORBA orb
	CORBA::ORB_var orb = CORBA::ORB_init (argc, argv);

	
	// Get the root POA
	CORBA::Object_var pfobj = orb->resolve_initial_references("RootPOA");
	PortableServer::POA_var rootPOA =
        PortableServer::POA::_narrow(pfobj);

	// Activate the root POA's manager
	PortableServer::POAManager_var mgr = rootPOA->the_POAManager();
	mgr->activate();

	// Create a Servant and explicitly create a CORBA object
	hellomodule::Hello_impl servant;
	CORBA::Object_var object = servant._this();

	// Here we get the IOR for the Hello server object.
	// Our "client" will use the IOR to find the object to connect to 
	CORBA::String_var ref = orb->object_to_string( object );

	// print out the IOR
	cout << ref << endl;

	// run the server event loop
	orb->run();

}
