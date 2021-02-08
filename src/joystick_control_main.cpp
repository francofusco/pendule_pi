#include <iostream>
#include "pendule_pi/version.hpp"

int main(int argc, char** argv) {

  std::cout << "Hello Version " << pendule_pi::getVersion() << std::endl;

  return 0;
}
