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
	
	Test::long_seq in_seq;
	in_seq.length (5);
	for (CORBA::ULong i = 0; i < 5; i++)
	{
		in_seq[i] = i * 11;
	}

	master_ptr->seq_in (in_seq);

	///////////////////////////////////
	// <-> INOUT
	std::cerr << "client::inout 1" << std::endl;
	
	Test::long_seq inout_seq;
	inout_seq.length (5);
	for (CORBA::ULong i = 0; i < 5; i++)
	{
		inout_seq[i] = (i + 1) * 22;
	}
	
	std::cerr << "client::inout 2" << std::endl;
	master_ptr->seq_inout (inout_seq);
	
	for (CORBA::ULong i = 0; i < inout_seq.length (); i++)
		std::cout << "Client::seq_inout[" << i << "] == " << inout_seq[i] << std::endl;
	

	///////////////////////////////////
	// <- OUT
	Test::long_seq_var out_seq;
	master_ptr->seq_out (out_seq);

	for (CORBA::ULong i = 0; i < out_seq->length (); i++)
		std::cout << "Client::seq_out[" << i << "] == " << out_seq[i] << std::endl;
	

	///////////////////////////////////
	// <- RET
	Test::long_seq_var ret_seq = master_ptr->seq_ret ();

	for (CORBA::ULong i = 0; i < ret_seq->length (); i++)
		std::cout << "Client::seq_ret[" << i << "] == " << ret_seq[i] << std::endl;
	
	
	return 0;
}
