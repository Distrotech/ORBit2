/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 4 -*- */

#include "test-cpp-stubs.hh"
#include <iostream>
#include <fstream>
#include <string>
	

void testLongs(Test::IMaster_ptr master_ptr);
void testStrings(Test::IMaster_ptr master_ptr);


int main(int argc, char *argv[])
{
	CORBA::ORB_var orb = CORBA::ORB_init(argc, argv, "orbit-local-orb");
	
	std::ifstream ior_stream ("test.ior");
	std::string ior;
	ior_stream >> ior;
	
	CORBA::Object_var obj = orb->string_to_object (ior);
	Test::IMaster_var master_ptr = Test::IMaster::_narrow (obj);

	testLongs(master_ptr);
	testStrings(master_ptr);

	return 0;
}


void testLongs(Test::IMaster_ptr master_ptr)
{

	///////////////////////////////////
	// -> IN
	
	Test::long_seq in_seq;
	in_seq.length (5);
	for (CORBA::ULong i = 0; i < 5; i++)
	{
		in_seq[i] = i * 11;
	}

	master_ptr->longseq_in (in_seq);

	///////////////////////////////////
	// <-> INOUT
	
	Test::long_seq inout_seq;
	inout_seq.length (5);
	for (CORBA::ULong i = 0; i < 5; i++)
	{
		inout_seq[i] = (i + 1) * 22;
	}
	
	master_ptr->longseq_inout (inout_seq);
	
	for (CORBA::ULong i = 0; i < inout_seq.length (); i++)
		std::cout << "Client::seq_inout[" << i << "] == " << inout_seq[i] << std::endl;
	

	///////////////////////////////////
	// <- OUT
	Test::long_seq_var out_seq;
	master_ptr->longseq_out (out_seq);

	for (CORBA::ULong i = 0; i < out_seq->length (); i++)
		std::cout << "Client::seq_out[" << i << "] == " << out_seq[i] << std::endl;
	

	///////////////////////////////////
	// <- RET
	Test::long_seq_var ret_seq = master_ptr->longseq_ret ();

	for (CORBA::ULong i = 0; i < ret_seq->length (); i++)
		std::cout << "Client::seq_ret[" << i << "] == " << ret_seq[i] << std::endl;
}


void testStrings(Test::IMaster_ptr master_ptr)
{
	///////////////////////////////////
	// -> IN
	
	Test::string_seq in_seq;
	in_seq.length (5);
	in_seq[0] = "String #1 in client sequence";
	in_seq[1] = "String #2 in client sequence";
	in_seq[2] = "String #3 in client sequence";
	in_seq[3] = "String #4 in client sequence";
	in_seq[4] = "String #5 in client sequence";

	master_ptr->strseq_in (in_seq);

	///////////////////////////////////
	// <-> INOUT
	
	Test::string_seq inout_seq;
	inout_seq.length (5);
	inout_seq[0] = "String #1 in client sequence";
	inout_seq[1] = "String #2 in client sequence";
	inout_seq[2] = "String #3 in client sequence";
	inout_seq[3] = "String #4 in client sequence";
	inout_seq[4] = "String #5 in client sequence";
	
	master_ptr->strseq_inout (inout_seq);
	
	for (CORBA::ULong i = 0; i < inout_seq.length (); i++)
		std::cout << "Client::seq_inout[" << i << "] == " << inout_seq[i] << std::endl;
	

	///////////////////////////////////
	// <- OUT
	Test::string_seq_var out_seq;
	master_ptr->strseq_out (out_seq);

	for (CORBA::ULong i = 0; i < out_seq->length (); i++)
		std::cout << "Client::seq_out[" << i << "] == " << out_seq[i] << std::endl;
	

	///////////////////////////////////
	// <- RET
	Test::string_seq_var ret_seq = master_ptr->strseq_ret ();

	for (CORBA::ULong i = 0; i < ret_seq->length (); i++)
		std::cout << "Client::seq_ret[" << i << "] == " << ret_seq[i] << std::endl;
}
