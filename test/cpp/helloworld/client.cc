/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */
#include <iostream>
#include "helloworld-cpp-stubs.hh"

using namespace std;
	
int
main(int argc, char *argv[])
{
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "orbit-local-orb");
	CORBA::Object_var obj = orb->string_to_object(argv[1]);
	hellomodule::Hello_var ptr = hellomodule::Hello::_narrow(obj);

	const char* message = "Hello server, from client!";
	const char* reply = ptr->helloWorld(message);

	cout << "Client: Reply was \"" << reply << "\"" << endl;
	
	return 0;  // pass
}
