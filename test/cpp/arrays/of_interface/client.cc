/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "test-cpp-stubs.hh"
#include <iostream>
#include <fstream>
#include <string>
	
int main(int argc, char *argv[])
{
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "orbit-local-orb");
	
	std::ifstream ior_stream ("test.ior");
	std::string ior;
	ior_stream >> ior;
	
	CORBA::Object_var obj = orb->string_to_object (ior);
	Test::IMaster_var master_ptr = Test::IMaster::_narrow (obj);
	
	/////////////////////
	// Call simple_ret
	Test::IMember_var member_ptr = master_ptr->simple_ret ();
	member_ptr->print_name ();

	/////////////////////
	// Call simple_in
	master_ptr->simple_in (member_ptr);
	

	//////////////////////////////////////////////////////////////
	// Arrays
	
	/////////////////////
	// Call array_ret
	Test::ObjArray_var array_ret;
	
	array_ret = master_ptr->array_ret ();
	for (int i = 0; i < 3; i++)
	{
		array_ret[i]->print_name ();
	}

	/////////////////////
	// Call array_in
	master_ptr->array_in (array_ret);

	return 0;
}
