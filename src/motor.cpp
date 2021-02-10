#include "pendule_pi/motor.hpp"
#include "pendule_pi/pigpio.hpp"
#include <pigpio.h>



namespace pendule_pi {


Motor::Motor(
  int pwm_pin,
  int direction_pin
)
: pwm_pin_(pwm_pin)
, dir_pin_(direction_pin)
, pwm_(0)
{
  // Set the direction pin in output mode. No need to do it with the PWM.
  PiGPIO_RUN_VOID(gpioSetMode, dir_pin_, PI_OUTPUT);
  // Make sure the PWM starts at zero.
  PiGPIO_RUN_VOID(gpioPWM, pwm_pin_, 0);
}


Motor::~Motor() {
  // write a zero on the speed pin
  gpioPWM(pwm_pin_, 0);
  // set all pins in high impedance, just in case
  gpioSetPullUpDown(pwm_pin_, PI_PUD_OFF);
  gpioSetPullUpDown(dir_pin_, PI_PUD_OFF);
  gpioSetMode(pwm_pin_, PI_INPUT);
  gpioSetMode(dir_pin_, PI_INPUT);
}


void Motor::setPWM(
  int pwm
)
{
  if(pwm > 0) {
    PiGPIO_RUN_VOID(gpioWrite, dir_pin_, PI_HIGH);
    PiGPIO_RUN_VOID(gpioPWM, pwm_pin_, pwm);
  }
  else {
    PiGPIO_RUN_VOID(gpioWrite, dir_pin_, PI_LOW);
    PiGPIO_RUN_VOID(gpioPWM, pwm_pin_, -pwm);
  }
  pwm_ = pwm;
}

}
