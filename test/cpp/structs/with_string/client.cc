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
	Test::Test_st struct_in;
	struct_in.num = 42;
	struct_in.string_one = CORBA::string_dup ("This is string #1");
	struct_in.string_two = CORBA::string_dup ("IN string #2");
	
	master_ptr->struct_in (struct_in);
	
	///////////////////////////////////
	// <-> INOUT
	master_ptr->struct_inout (struct_in);
    std::cerr << "Client::struct_inout: num = " << struct_in.num << std::endl;
	std::cerr << "Client::struct_inout: one = " << struct_in.string_one << std::endl;
    std::cerr << "Client::struct_inout: two = " << struct_in.string_two << std::endl;

	///////////////////////////////////
	// <- OUT
	Test::Test_st_var struct_out;
	master_ptr->struct_out (struct_out);

    std::cerr << "Client::struct_out: num = " << struct_out->num << std::endl;
	std::cerr << "Client::struct_out: one = " << struct_out->string_one << std::endl;
    std::cerr << "Client::struct_out: two = " << struct_out->string_two << std::endl;

	///////////////////////////////////
	// <- RET
	Test::Test_st_var struct_ret = master_ptr->struct_ret ();

    std::cerr << "Client::struct_ret: num = " << struct_ret->num << std::endl;
	std::cerr << "Client::struct_ret: one = " << struct_ret->string_one << std::endl;
    std::cerr << "Client::struct_ret: two = " << struct_ret->string_two << std::endl;


	return 0;
}
