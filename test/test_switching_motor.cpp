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
  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the Motor and Switch instances
    pp::Motor motor(24, 16);
    pp::Switch right(18, pp::Switch::NORMALLY_UP, pp::Switch::WITH_PULL_RESISTOR);
    pp::Switch left(17, pp::Switch::NORMALLY_UP, pp::Switch::WITH_PULL_RESISTOR);
    // If both switches are pressed, exit
    if(!right.atRest() && !left.atRest()) {
      std::cout << "Both switches are active, I do not know how to choose my initial direction..." << std::endl;
      return 0;
    }
    // Assign switch callbacks so that the motor changes its direction when
    // hitting any of the two switches.
    left.enableInterrupts([&](){motor.setPWM(30);});
    right.enableInterrupts([&](){motor.setPWM(-30);});
    // Set the initial PWM
    motor.setPWM( right.atRest() ? 30 : -30 );
    // Dummy loop that will never exit. Use CTRL+C to stop!
    while(true)
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
