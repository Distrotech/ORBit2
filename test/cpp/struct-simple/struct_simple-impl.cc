#include "struct_simple-impl.hh"
#include <iostream>

namespace Test
{

void TestIface_impl::test_in (const SimpleStructArg &in_struct,
			      const char            *message) throw (CORBA::SystemException)
{
    std::cout << "TestIface_Impl: Message is \"" << message << "\"" << std::endl;

#if 1
#warning "FIXME: This segfaults -- there's a refcounting/connection managment issue lurking around"
    for (int i = 0; i < in_struct.number; i++)
	in_struct.stream->print (message);
#else
    std::cout << "TestIFace_Impl: in_struct.number == " << in_struct.number << std::endl;
    in_struct.stream->print (message);
#endif
}


OutputStream_impl::OutputStream_impl (const std::string &member_data_):
    member_data (member_data_)
{
}

void OutputStream_impl::print (const char *message) throw (CORBA::SystemException)
{
    std::cout << "OutputStream server (" << member_data << "): \"" << message << "\"" << std::endl;}

} // namespace hellomodule
