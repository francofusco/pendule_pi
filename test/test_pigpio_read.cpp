#include "pendule_pi/pigpio.hpp"
#include <pigpio.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <cassert>


int main(int argc, char** argv) {
  try {
    // select the pin to be used
    int pin = argc==1 ? 0 : std::atoi(argv[1]);
    std::cout << "I will monitor pin " << pin << std::endl;
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // the pin will be used as input
    PiGPIO_RUN_VOID(gpioSetMode, pin, PI_INPUT);
    // read the current state of the pin
    int current,initial;
    PiGPIO_RUN(initial, gpioRead, pin);
    // this loop will exit after 10s or if the target pin changes its state
    for(unsigned int i=0; i<50; i++) {
      std::this_thread::sleep_for(std::chrono::milliseconds(200));
      PiGPIO_RUN(current, gpioRead, pin);
      if(current != initial) {
        std::cout << "Switch hit, exiting" << std::endl;
        return 0;
      }
    }
    std::cout << "Max wait-time elapsed, exiting" << std::endl;
  }
  catch(const pigpio::ActivationToken::Terminate&) { }
  catch(...) { throw; }

  return 0;
}
