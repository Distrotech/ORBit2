#ifndef TEST_IMPL_HH
#define TEST_IMPL_HH

#include "test-cpp-skels.hh"

class IMaster_impl: public POA_Test::IMaster
{
	char *str1;
	char *str2;
	char *str3;

public:
	IMaster_impl();

	void longseq_in (const ::Test::long_seq &seq_in)
		throw (CORBA::SystemException);
	void longseq_inout (::Test::long_seq &seq_inout)
		throw (CORBA::SystemException);
	void longseq_out (::Test::long_seq_out seq_out)
		throw (CORBA::SystemException);
	::Test::long_seq* longseq_ret ()
		throw (CORBA::SystemException);

	void strseq_in (const ::Test::string_seq &seq_in)
		throw (CORBA::SystemException);
	void strseq_inout (::Test::string_seq &seq_inout)
		throw (CORBA::SystemException);
	void strseq_out (::Test::string_seq_out seq_out)
		throw (CORBA::SystemException);
	::Test::string_seq* strseq_ret ()
		throw (CORBA::SystemException);
};

#endif
