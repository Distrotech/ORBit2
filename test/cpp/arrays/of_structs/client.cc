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

	///////////////////////////////////
	// -> IN
	Test::TestArray in_array;

	in_array[0].num = 10;
	in_array[0].string_one = CORBA::string_dup ("IN string #1");
	in_array[0].string_two = CORBA::string_dup ("IN string #2");

	in_array[1].num = 20;
	in_array[1].string_one = CORBA::string_dup ("IN #2 string #1");
	in_array[1].string_two = CORBA::string_dup ("IN #2 string #2");
	
	master_ptr->array_in (in_array);
	
	
#if 0	
	///////////////////////////////////
	// <- RET
	Test::Outer_st_var struct_ret = master_ptr->struct_ret ();
	
	std::cerr << "Client::RET: num = "
			  << struct_ret->num << std::endl;
	std::cerr << "Client::RET: id = "
			  << struct_ret->id << std::endl;
	
	std::cerr << "Client::RET: struct_member.num = "
			  << struct_ret->struct_member->num << std::endl;
	std::cerr << "Client::RET: struct_member.string_one = "
			  << struct_ret->struct_member->string_one << std::endl;
	std::cerr << "Client::RET: struct_member.string_two = "
			  << struct_ret->struct_member->string_two << std::endl;
#endif
	
	return 0;
}
