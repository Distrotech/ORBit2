#include "helloworld-impl.hh"
#include <iostream>

char*
hellomodule::Hello_impl::helloWorld(const char* greeting) throw(CORBA::SystemException)
{
  std::cout << "Server: Greeting was \"" << greeting << "\"" << std::endl;
  return CORBA::string_dup("Hello client, from server!");
}
