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

	try {
		master_ptr->provoke ();
	} catch (Test::ExTest &ex)
	{
		std::cerr << "Test::ExTest caught!" << std::endl;

		std::cerr << "Test::ExTest::nums.num_1 = " << ex.nums.num_1 << std::endl;
		std::cerr << "Test::ExTest::nums.num_2 = " << ex.nums.num_2 << std::endl;
		
		ex.objs.member_1->print_name ();
		ex.objs.member_2->print_name ();
	}

	return 0;
}
