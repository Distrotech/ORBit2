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
	// <- RET
	Test::Test_st_var strct = master_ptr->struct_ret ();

    std::cerr << "Client::struct_ret: num = " << strct->num << std::endl;
	strct->obj_one->print_name ();
	strct->obj_two->print_name ();

	///////////////////////////////////
	// -> IN
	strct->num = 42;
	
	master_ptr->struct_in (strct);
	
	///////////////////////////////////
	// <-> INOUT
	master_ptr->struct_inout (strct);
	master_ptr->struct_in (strct);

	///////////////////////////////////
	// <- OUT
	Test::Test_st_var struct_out;
	master_ptr->struct_out (struct_out);

    std::cerr << "Client::struct_out: num = " << struct_out->num << std::endl;
	struct_out->obj_one->print_name ();
	struct_out->obj_two->print_name ();
	
	return 0;
}
