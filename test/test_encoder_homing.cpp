#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/switch.hpp"
#include "pendule_pi/motor.hpp"
#include "pendule_pi/encoder.hpp"
#include <pigpio.h>
#include <iostream>
#include <chrono>
#include <thread>


int main(int argc, char** argv) {
  namespace pp = pendule_pi;
  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the Motor and Switch instances
    pp::Motor motor(24, 16);
    pp::Switch right(18, true);
    pp::Switch left(17, true);
    // Create the encoder
    pp::Encoder encoder(20, 21);
    // If both switches are pressed, exit
    if(!right.atRest(true) && !left.atRest(true)) {
      std::cout << "Both switches are active, I do not know how to choose my initial direction..." << std::endl;
      return 0;
    }
    // Assign switch callbacks so that the motor changes its direction when
    // hitting any of the two switches.
    bool switched = false;
    const int INITIAL_SPEED = 50;
    const int HOMING_SPEED = 150;
    const int HOMING_SPEED_FINE = 30;
    left.enableInterrupts([&](){motor.setPWM(HOMING_SPEED); switched=true;});
    right.enableInterrupts([&](){motor.setPWM(-HOMING_SPEED); switched=true;});
    // Set the initial PWM
    motor.setPWM( right.atRest(true) ? INITIAL_SPEED : -INITIAL_SPEED );
    // Dummy loop that waits until a switch is hit
    while(!switched)
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // We reach this point if a switch was hit; we then enter a loop that
    // waits for the encoder to reach the zero position.
    while(std::abs(encoder.steps()) > 3000)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    int speed = (motor.getPWM()>0?1:-1) * HOMING_SPEED_FINE;
    motor.setPWM(speed);
    while(std::abs(encoder.steps()) > 50)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
