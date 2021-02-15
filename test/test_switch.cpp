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
    // select the pin to be used
    int pin = argc==1 ? 17 : std::atoi(argv[1]);
    // create the Switch object
    pp::Switch sw(pin, true);
    // Connect a custom callback that will be executed everytime that
    // the switch gets activated. In this case, just print a message.
    sw.enableInterrupts([](){std::cout << "Pressed" << std::endl;});
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));
    std::cout << "Max wait-time elapsed, exiting" << std::endl;
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
