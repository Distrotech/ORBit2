/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "helloworld-cpp-stubs.hh"
#include <iostream>
	
int
main(int argc, char *argv[])
{
  if(argc < 2)
  {
    g_warning("usage:\n  client id\n");
  }
  else
  {
    try
    {
   	  CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "orbit-local-orb");

   	  CORBA::Object_var obj = orb->string_to_object(argv[1]);

  	  hellomodule::Hello_var ptr = hellomodule::Hello::_narrow(obj);

  	  const char* message = "Hello server, from client!";
  	  const char* reply = ptr->helloWorld(message);

  	  std::cout << "Client: Reply was \"" << reply << "\"" << std::endl;
    }
    catch(const CORBA::Exception& ex)
    {
       std::cout << "Exception caught. Maybe the server is not running, or the id is wrong." << std::endl;
    }

  } //if
	
	return 0;  // pass
}
