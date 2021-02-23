#include "pendule_pi/joystick.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

int main() {
  namespace pp = pendule_pi;
  try {
    std::cout << std::setfill('0') << std::internal;
    pp::Joystick joy;

    while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
      joy.update();
      std::cout << "\r";
      // print button states
      std::cout << std::noshowpos;
      for(unsigned int i=0; i<joy.nButtons(); i++)
        std::cout << "  " << std::setw(1) << joy.button(i);
      // print axes states
      std::cout << std::showpos;
      for(unsigned int i=0; i<joy.nAxes(); i++)
        std::cout << "  " << std::setw(6) << joy.axis(i);
      std::cout << "  " << std::flush;
    }
  }
  catch(...) { throw; }

  return 0;
}
