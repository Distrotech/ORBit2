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
		for (CORBA::Long i = 0; i < 3; i++)
		{
			std::cerr << "Test::ExTest::nums[" << i << "] = "
					  << ex.nums[i] << std::endl;
			ex.objs[i]->print_name ();
		}
	}

	return 0;
}
