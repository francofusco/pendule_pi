#include <pigpio.h>
#include <iostream>

#define RUN_THEN_PRINT(func, ...) { auto retval = func(__VA_ARGS__); std::cout << #func << "(" << #__VA_ARGS__ << ")" << " returned " << retval << std::endl; }

int main(int argc, char** argv) {
  std::cout << "GPIO init: " << gpioInitialise() << std::endl;

  for(int i=0; i<=26; i++) {
    std::cout << "GPIO " << i << std::endl;
    RUN_THEN_PRINT(gpioSetMode, i, PI_PUD_OFF);
    RUN_THEN_PRINT(gpioSetMode, i, PI_INPUT);
  }

  gpioTerminate();
  return 0;
}
