#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/switch.hpp"
#include <pigpio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>


int main(int argc, char** argv) {
  namespace pp = pendule_pi;
  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // create a Switch object
    pp::Switch sw(17, true);
    // Try to throw inside a GPIO callback
    sw.enableInterrupts([](){throw std::runtime_error("Throw exception on GPIO callback");});
    // Wait for the switch to be hit
    while(true) {
      std::cout << "Hit the switch to fire the exception" << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
