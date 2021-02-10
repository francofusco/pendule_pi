#include "pendule_pi/pigpio.hpp"
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
    // Create the DC motor
    pp::Motor motor(24, 16);
    // move to the right
    motor.setPWM(30);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // move to the left
    motor.setPWM(-30);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // stop moving
    motor.setPWM(0);
  }
  catch(const pigpio::ActivationToken::Terminate&) { }
  catch(...) { throw; }

  return 0;
}
