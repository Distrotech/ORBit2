#include <iostream>
#include "helloworld-impl.hh"


char*
hellomodule::Hello_impl::helloWorld(const char* greeting)
{
  cout << "Server: Greeting was \"" << greeting << "\"" << endl;
  return CORBA::string_dup("Hello client, from server!");
}
