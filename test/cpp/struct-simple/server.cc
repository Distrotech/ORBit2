/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "struct_simple-impl.hh"
#include <iostream>
#include <fstream>

int main (int argc, char* argv[])
{
  try
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
 	  Test::TestIface_impl test_iface_servant;
 	  CORBA::Object_var test_iface = test_iface_servant._this();

 	  Test::OutputStream_impl stream_servant ("Instance `foo'");
 	  CORBA::Object_var stream = stream_servant._this();

 	  // Here we get the IOR for the Hello server object.
 	  // Our "client" will use the IOR to find the object to connect to
 	  CORBA::String_var test_iface_ior = orb->object_to_string (test_iface);
 	  CORBA::String_var stream_ior = orb->object_to_string (stream);
	  
 	  // print out the IORs to files
	  std::ofstream test_iface_ior_file ("test_iface.ior");
 	  test_iface_ior_file << test_iface_ior << std::endl;

	  std::ofstream stream_ior_file ("stream.ior");
 	  stream_ior_file << stream_ior << std::endl;
	  
 	  // run the server event loop
 	  orb->run();
  }
  catch(const CORBA::Exception& ex)
  {
    std::cout << "Exception caught." << std::endl;
  }

}
