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
	struct_in.one = 1;
	struct_in.two = 2;
	struct_in.dir = Test::DIR_WEST;
	
	master_ptr->struct_in (struct_in);

	
	///////////////////////////////////
	// <-> INOUT
	master_ptr->struct_inout (struct_in);
	std::cerr << "Client::struct_inout: one = " << struct_in.one << std::endl;
    std::cerr << "Client::struct_inout: two = " << struct_in.two << std::endl;
    std::cerr << "Client::struct_inout: dir = " << struct_in.dir << std::endl;

	///////////////////////////////////
	// <- OUT
	Test::Test_st struct_out;
	master_ptr->struct_out (struct_out);

	std::cerr << "Client::struct_out: one = " << struct_out.one << std::endl;
    std::cerr << "Client::struct_out: two = " << struct_out.two << std::endl;
    std::cerr << "Client::struct_out: dir = " << struct_out.dir << std::endl;

	///////////////////////////////////
	// <- RET
	Test::Test_st struct_ret = master_ptr->struct_ret ();

	std::cerr << "Client::struct_ret: one = " << struct_ret.one << std::endl;
    std::cerr << "Client::struct_ret: two = " << struct_ret.two << std::endl;
    std::cerr << "Client::struct_ret: dir = " << struct_ret.dir << std::endl;
	
	return 0;
}
