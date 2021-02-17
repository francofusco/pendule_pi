#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/switch.hpp"
#include "pendule_pi/motor.hpp"
#include <pigpio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>


int main(int argc, char** argv) {
  namespace pp = pendule_pi;
  bool start_motor = (argc>1 && std::string(argv[1])=="--motor");
  if(!start_motor) {
    std::cout << "The motor will NOT be started. To start it, use " << argv[0] << " --motor" << std::endl;
    std::cout << "However, be cautious since this might break your hardware!" << std::endl;
  }

  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // create a Switch object
    pp::Switch sw(17, pp::Switch::NORMALLY_UP, pp::Switch::WITH_PULL_RESISTOR);
    // create a motor
    pp::Motor motor(24, 16);
    if(start_motor)
      motor.setPWM(30);
    // WARNING: THE MOTOR WILL KEEP SPINNING
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
