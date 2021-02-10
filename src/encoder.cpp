#include "pendule_pi/encoder.hpp"
#include "pendule_pi/pigpio.hpp"
#include <pigpio.h>



namespace pendule_pi {


Encoder::Encoder(
  int gpioA,
  int gpioB
)
: pin_a_(gpioA)
, pin_b_(gpioB)
{
   // TODO: Should I init the value in a better way?
   level_a_ = 0;
   level_b_ = 0;
   // TODO: check if the following is a good initialization
   last_triggered_ = -1;

   // Try configuring the pins using pigpio
   gpioSetMode(pin_a_, PI_INPUT);
   gpioSetMode(pin_b_, PI_INPUT);

   /* pull up is needed as encoder common is grounded */

   gpioSetPullUpDown(pin_a_, PI_PUD_UP);
   gpioSetPullUpDown(pin_b_, PI_PUD_UP);

   /* monitor encoder level changes */

   gpioSetAlertFuncEx(pin_a_, Encoder::pulseStatic, this);
   gpioSetAlertFuncEx(pin_b_, Encoder::pulseStatic, this);

}


Encoder::~Encoder() {
  // gpioSetAlertFuncEx(mygpioB, nullptr, nullptr);
  // gpioSetAlertFuncEx(mygpioA, nullptr, nullptr);
}

void Encoder::pulse(int gpio, int level, unsigned int tick)
{
  throw 0;
   // if (gpio == mygpioA) levA = level; else levB = level;
   //
   // if (gpio != lastGpio) /* debounce */
   // {
   //    lastGpio = gpio;
   //
   //    if ((gpio == mygpioA) && (level == 1))
   //    {
   //       if (levB) (mycallback)(1);
   //    }
   //    else if ((gpio == mygpioB) && (level == 1))
   //    {
   //       if (levA) (mycallback)(-1);
   //    }
   // }
}

void Encoder::pulseStatic(
  int gpio,
  int level,
  unsigned int tick,
  void* THIS
)
{
   static_cast<Encoder*>(THIS)->pulse(gpio, level, tick);
}



}
