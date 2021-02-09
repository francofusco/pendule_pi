#include "pendule_pi/encoder.hpp"
#include <pigpio.h>


namespace pendule_pi {


Encoder::Encoder(
  int gpioA,
  int gpioB
)
: pin_a_(gpioA)
, pin_b_(gpioB)
{
   mygpioA = gpioA;
   mygpioB = gpioB;

   mycallback = callback;

   // TEMP TODO: read current value!!!
   level_a_ = 0;
   level_b_ = 0;

   last_triggered_ = -1;

   gpioSetMode(gpioA, PI_INPUT);
   gpioSetMode(gpioB, PI_INPUT);

   /* pull up is needed as encoder common is grounded */

   gpioSetPullUpDown(gpioA, PI_PUD_UP);
   gpioSetPullUpDown(gpioB, PI_PUD_UP);

   /* monitor encoder level changes */

   gpioSetAlertFuncEx(gpioA, _pulseEx, this);
   gpioSetAlertFuncEx(gpioB, _pulseEx, this);
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
