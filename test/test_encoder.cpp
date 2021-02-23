#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/switch.hpp"
#include "pendule_pi/motor.hpp"
#include "pendule_pi/encoder.hpp"
#include <pigpio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>


int main() {
  namespace pp = pendule_pi;
  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the Motor and Switch instances
    pp::Motor motor(24, 16);
    pp::Switch right(18, pp::Switch::NORMALLY_UP, pp::Switch::WITH_PULL_RESISTOR);
    pp::Switch left(17, pp::Switch::NORMALLY_UP, pp::Switch::WITH_PULL_RESISTOR);
    // Create the encoder
    pp::Encoder encoder(20, 21);
    // If both switches are pressed, exit
    if(!right.atRest() && !left.atRest()) {
      std::cout << "Both switches are active, I do not know how to choose my initial direction..." << std::endl;
      return 0;
    }
    // Assign switch callbacks so that the motor changes its direction when
    // hitting any of the two switches.
    const int SPEED = 50;
    left.enableInterrupts([&](){motor.setPWM(SPEED);});
    right.enableInterrupts([&](){motor.setPWM(-SPEED);});
    // Set the initial PWM
    motor.setPWM( right.atRest() ? SPEED : -SPEED );
    // Dummy loop that will never exit. Use CTRL+C to stop!
    const std::map<int,std::string> direction = {{-1,"left "},{0,"stale"},{1,"right"}};
    while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(25));
      int dir = encoder.direction();
      std::cout << "Direction: " << direction.at(dir) << " / Position: " << encoder.steps() << std::endl;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
