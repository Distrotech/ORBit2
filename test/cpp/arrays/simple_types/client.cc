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
	
	CORBA::Object_var obj = orb->string_to_object(ior);		
	Test::IFace_var iface_ptr = Test::IFace::_narrow(obj);
	
	/////////////////////
	// Call array_method
	Test::LongArray          long_array;
	Test::StringArray_slice *string_array;
	
	iface_ptr->array_method (long_array, string_array);

	for (int i = 0; i < 10; i++)
		std::cout << "Client:\tlong_array[" << i << "] == " << long_array[i]
				  << std::endl;

	for (int i = 0; i < 10; i++)
		std::cout << "Client:\tstring_array[" << i << "] == "
				  << '"' << string_array[i] << '"' << std::endl;
	Test::StringArray_free (string_array);

	/////////////////////
	// Call strings_method
	Test::StringArray in_array;
	Test::StringArray inout_array;
	gchar *tmp;
	for (int i = 0; i < 10; i++)
	{
		tmp = g_strdup_printf ("IN data #%d", i + 1);
		in_array[i] = CORBA::string_dup (tmp);
		g_free (tmp);

		tmp = g_strdup_printf ("INOUT data #%d", i + 1);
		inout_array[i] = CORBA::string_dup (tmp);
		g_free (tmp);
	}

	iface_ptr->strings_method (in_array, inout_array);

	for (int i = 0; i < 10; i++)
		std::cout << "Client: INOUT: " << inout_array[i] << std::endl;
	
	/////////////////////
	// Call strings_ret
	Test::StringArray_slice * ret_array;

	ret_array = iface_ptr->strings_ret ();

	for (int i = 0; i < 10; i++)
		std::cout << "Client: RET: " << ret_array[i] << std::endl;
	Test::StringArray_free (ret_array);
	
	return 0;
}
