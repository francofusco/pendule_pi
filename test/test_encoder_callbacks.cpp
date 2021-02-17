#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/switch.hpp"
#include "pendule_pi/motor.hpp"
#include "pendule_pi/encoder.hpp"
#include <pigpio.h>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>


int main(int argc, char** argv) {
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
    // If any of the switches is pressed, exit
    if(!right.atRest() || !left.atRest()) {
      std::cout << "Both switches must be inactive!" << std::endl;
      return 0;
    }
    const int LOW_SPEED = 50;
    const int HIGH_SPEED = 250;
    const int THRESHOLD_MARGIN = 20000;
    bool both_switches_hit = false;
    int upper_threshold, lower_threshold;
    // Move the base until the right switch is hit, then go to the left.
    // Let interrupts do the magic.
    right.enableInterrupts([&](){
      motor.setPWM(-LOW_SPEED);
      upper_threshold = encoder.steps();
    });
    left.enableInterrupts([&](){
      motor.setPWM(LOW_SPEED);
      lower_threshold = encoder.steps();
      both_switches_hit = true;
    });
    // Set the initial PWM
    motor.setPWM(LOW_SPEED);
    // Dummy loop that waits until both switches are hit
    while(!both_switches_hit)
      std::this_thread::sleep_for(std::chrono::milliseconds(50));
    lower_threshold += THRESHOLD_MARGIN;
    upper_threshold -= THRESHOLD_MARGIN;
    if(lower_threshold >= upper_threshold)
      throw std::runtime_error("Lower threshold seems larger than the upper one, this makes no sense!");
    // go to the center of the rail
    int middle_position = (lower_threshold + upper_threshold) / 2;
    while(encoder.steps() < middle_position)
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    motor.setPWM(0);
    // setup safety thresholds
    right.enableInterrupts([&](){
      throw std::runtime_error("Right switch hit!");
    });
    left.enableInterrupts([&](){
      throw std::runtime_error("Left switch hit!");
    });
    encoder.setSafetyCallbacks(
      lower_threshold,
      upper_threshold,
      [&](){ motor.setPWM(HIGH_SPEED); },
      [&](){ motor.setPWM(-HIGH_SPEED); }
    );
    // start the motor
    motor.setPWM(HIGH_SPEED);
    while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      auto pos = encoder.steps();
      std::string sig = pos >= 0 ? "+" : "-";
      pos = std::abs(pos);
      std::cout << "\rPosition: " << sig << std::setw(7) << std::setfill('0') << pos << std::flush;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
