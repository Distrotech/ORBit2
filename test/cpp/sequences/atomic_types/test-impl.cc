#include "test-impl.hh"

IMaster_impl::IMaster_impl()
	: str1("String #1 in server sequence"),
	str2("String #2 in server sequence"),
	str3("String #3 in server sequence")
{

}


void IMaster_impl::longseq_in(const ::Test::long_seq &in_seq)
	throw (CORBA::SystemException)
{
	for (CORBA::ULong i = 0; i < in_seq.length(); i++) {
		std::cout << "Server::in_seq[" << i << "] == "
			<< in_seq[i] << std::endl;
	}
}


void IMaster_impl::longseq_inout(::Test::long_seq &inout_seq)
	throw (CORBA::SystemException)
{
	for (CORBA::ULong i = 0; i < inout_seq.length(); i++) {
		std::cout << "Server::inout_seq[" << i << "] == "
			<< inout_seq[i] << std::endl;
	}

	inout_seq.length(4);
	for (CORBA::ULong i = 0; i < 4; i++) {
		inout_seq[i] = i * 13;
	}
}


void IMaster_impl::longseq_out (::Test::long_seq_out out_seq)
	throw (CORBA::SystemException)
{
	CORBA::Long *buf = ::Test::long_seq::allocbuf(6);
	for (CORBA::ULong i = 0; i < 6; i++) {
		buf[i] = (i * 13) - 12;
	}

	out_seq = new ::Test::long_seq(6, 6, buf, TRUE);
}


::Test::long_seq* IMaster_impl::longseq_ret ()
	throw (CORBA::SystemException)
{
	CORBA::Long *buf = ::Test::long_seq::allocbuf(3);

	for (CORBA::ULong i = 0; i < 3; i++) {
		buf[i] = (i * 2) - 12;
	}

	return new ::Test::long_seq(3, 3, buf, TRUE);
}


void IMaster_impl::strseq_in(const ::Test::string_seq &in_seq)
	throw (CORBA::SystemException)
{
	for (CORBA::ULong i = 0; i < in_seq.length(); i++) {
		std::cout << "server::seq_in[" << i << "] == "
			<< in_seq[i] << endl;
	}
}


void IMaster_impl::strseq_inout(::Test::string_seq &inout_seq)
	throw (CORBA::SystemException)
{
	int i = 0;

	for (i = 0; i < inout_seq.length(); i++) {
		std::cout << "server::seq_inout[" << i << "] == "
			<< inout_seq[i] << endl;
	}

	inout_seq.length(3);
	inout_seq[0] = CORBA::string_dup(str1);
	inout_seq[1] = CORBA::string_dup(str2);
	inout_seq[2] = CORBA::string_dup(str3);
}


void IMaster_impl::strseq_out(::Test::string_seq_out seq_out)
	throw (CORBA::SystemException)
{
	CORBA::String_mgr *buf = ::Test::string_seq::allocbuf(3);

	buf[0] = CORBA::string_dup(str1);
	buf[1] = CORBA::string_dup(str2);
	buf[2] = CORBA::string_dup(str3);

	seq_out = new ::Test::string_seq(3, 3, buf, TRUE);
}


::Test::string_seq* IMaster_impl::strseq_ret()
	throw (CORBA::SystemException)
{
	CORBA::String_mgr *buf = ::Test::string_seq::allocbuf(3);

	buf[0] = CORBA::string_dup(str1);
	buf[1] = CORBA::string_dup(str2);
	buf[2] = CORBA::string_dup(str3);

	return new ::Test::string_seq(3, 3, buf, TRUE);
}
