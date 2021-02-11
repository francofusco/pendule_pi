#include "pendule_pi/encoder.hpp"
#include "pendule_pi/pigpio.hpp"
#include <pigpio.h>



namespace pendule_pi {


const std::vector<int> Encoder::ENCODER_TABLE = {0,1,-1,0,-1,0,0,1,1,0,0,-1,0,-1,1,0};


int Encoder::encode(
  bool a,
  bool b,
  bool past_a,
  bool past_b
)
{
  return (past_a<<3) | (past_b<<2) | (a<<1) | (b);
}


Encoder::Encoder(
  int gpioA,
  int gpioB
)
: pin_a_(gpioA)
, pin_b_(gpioB)
, a_current_(0)
, b_current_(0)
, a_past_(0)
, b_past_(0)
, steps_(0)
{
  // Configure the pins using pigpio
  PiGPIO_RUN_VOID(gpioSetMode, pin_a_, PI_INPUT);
  PiGPIO_RUN_VOID(gpioSetMode, pin_b_, PI_INPUT);

  // This assumes that the phases are common grounded.
  PiGPIO_RUN_VOID(gpioSetPullUpDown, pin_a_, PI_PUD_UP);
  PiGPIO_RUN_VOID(gpioSetPullUpDown, pin_b_, PI_PUD_UP);

  // Setup interrupts so that we handle the rotation without polling.
  PiGPIO_RUN_VOID(gpioSetAlertFuncEx, pin_a_, Encoder::pulseStatic, this);
  PiGPIO_RUN_VOID(gpioSetAlertFuncEx, pin_b_, Encoder::pulseStatic, this);
}


Encoder::~Encoder() {
  // Disable interrupts
  gpioSetAlertFuncEx(pin_a_, nullptr, nullptr);
  gpioSetAlertFuncEx(pin_b_, nullptr, nullptr);
  // Set all pins in high impedance, just in case
  gpioSetPullUpDown(pin_a_, PI_PUD_OFF);
  gpioSetPullUpDown(pin_b_, PI_PUD_OFF);
  gpioSetMode(pin_a_, PI_INPUT);
  gpioSetMode(pin_b_, PI_INPUT);
}


void Encoder::pulse(int gpio, int level, unsigned int tick)
{
  // shift the pin levels as needed, depending on the pin that fired the interrupt
  if(gpio == pin_a_) {
    a_past_ = a_current_;
    a_current_ = level;
  }
  else if(gpio == pin_b_) {
    b_past_ = b_current_;
    b_current_ = level;
  }
  else {
    throw std::runtime_error(
      "Expected Encoder interrupt on pins " + std::to_string(pin_a_) + " or "
      + std::to_string(pin_b_) + ", however an interrupt was fired for pin "
      + std::to_string(gpio)
    );
  }

  // Update the current step count
  steps_ += ENCODER_TABLE.at(encode(a_current_, b_current_, a_past_, b_past_));
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
