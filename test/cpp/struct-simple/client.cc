/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "struct_simple-cpp-stubs.hh"
#include <iostream>
#include <fstream>
	
int
main(int argc, char *argv[])
{
    try
    {
		
   	  CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "orbit-local-orb");

	  std::ifstream test_iface_ior_file ("test_iface.ior");
	  std::string test_iface_ior;
	  test_iface_ior_file >> test_iface_ior;

	  std::ifstream stream_ior_file ("stream.ior");
	  std::string stream_ior;
	  stream_ior_file >> stream_ior;
	  
   	  CORBA::Object_var test_iface_obj = orb->string_to_object (test_iface_ior.c_str ());
   	  CORBA::Object_var stream_obj = orb->string_to_object (stream_ior.c_str ());
	  
  	  Test::TestIface_var test_iface_ptr = Test::TestIface::_narrow (test_iface_obj);
  	  Test::OutputStream_var stream_ptr = Test::OutputStream::_narrow (stream_obj);

	  std::cout << "Calling test_ret on remote object" << std::endl;
	  Test::SimpleStructArg *ret_struct = test_iface_ptr->test_ret ();
	  
  	  const char* message = "TestIface server, from client!";
	  Test::SimpleStructArg test_struct;
	  test_struct.stream = stream_ptr;
	  test_struct.number = ret_struct->number;
	  
	  std::cout << "Calling test_in on remote object" << std::endl;
	  test_iface_ptr->test_in (test_struct, message);

  	  //std::cout << "Client: Reply was \"" << reply << "\"" << std::endl;

	} catch (const CORBA::Exception& ex) {
		
       std::cerr << "Exception caught. "
				 << "Maybe the server is not running, "
				 << " or the id is wrong." << std::endl;
    }
	
	return 0;  // pass
}
