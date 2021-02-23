#include "pendule_pi/encoder.hpp"
#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/debug.hpp"
#include <pigpio.h>
#include <limits>



namespace pendule_pi {


const std::vector<int> Encoder::ENCODER_TABLE = {0,1,-1,0,-1,0,0,1,1,0,0,-1,0,-1,1,0};


int Encoder::encode(
  bool past_a,
  bool past_b,
  bool a,
  bool b
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
, lower_threshold_(0)
, upper_threshold_(0)
, lower_cb_(nullptr)
, upper_cb_(nullptr)
{
  PENDULE_PI_DBG("Creating Encoder on pins " << pin_a_ << " and " << pin_b_);
  // Configure the pins using pigpio
  PiGPIO_RUN_VOID(gpioSetMode, pin_a_, PI_INPUT);
  PiGPIO_RUN_VOID(gpioSetMode, pin_b_, PI_INPUT);

  // This assumes that the phases are common grounded.
  PiGPIO_RUN_VOID(gpioSetPullUpDown, pin_a_, PI_PUD_UP);
  PiGPIO_RUN_VOID(gpioSetPullUpDown, pin_b_, PI_PUD_UP);

  // Setup interrupts so that we handle the rotation without polling.
  PiGPIO_RUN_VOID(gpioSetAlertFuncEx, pin_a_, Encoder::pulseStatic, this);
  PiGPIO_RUN_VOID(gpioSetAlertFuncEx, pin_b_, Encoder::pulseStatic, this);

  // initialize properly the pin reading
  int level; // NOTE: use an int otherwise the comparison 'if(retval<0)' inside PiGPIO_RUN does not make sense!
  PiGPIO_RUN(level, gpioRead, pin_a_);
  a_current_ = a_past_ = level;
  PiGPIO_RUN(level, gpioRead, pin_b_);
  b_current_ = b_past_ = level;
  PENDULE_PI_DBG("Encoder created successfully");
}


Encoder::~Encoder() {
  PENDULE_PI_DBG("Destroying Encoder on pins " << pin_a_ << " and " << pin_b_);
  // Disable interrupts
  gpioSetAlertFuncEx(pin_a_, nullptr, nullptr);
  gpioSetAlertFuncEx(pin_b_, nullptr, nullptr);
  // Set all pins in high impedance, just in case
  gpioSetPullUpDown(pin_a_, PI_PUD_OFF);
  gpioSetPullUpDown(pin_b_, PI_PUD_OFF);
  gpioSetMode(pin_a_, PI_INPUT);
  gpioSetMode(pin_b_, PI_INPUT);
  PENDULE_PI_DBG("Encoder destroyed successfully");
}


void Encoder::setSafetyCallbacks(
  int lower_threshold,
  int upper_threshold,
  std::function<void(void)> lower_cb,
  std::function<void(void)> upper_cb
)
{
  lower_threshold_ = lower_threshold;
  upper_threshold_ = upper_threshold;
  lower_cb_ = lower_cb;
  upper_cb_ = upper_cb;
}


void Encoder::removeSafetyCallbacks() {
  lower_cb_ = nullptr;
  upper_cb_ = nullptr;
  lower_threshold_ = std::numeric_limits<int>::min();
  upper_threshold_ = std::numeric_limits<int>::max();
}


void Encoder::pulse(int gpio, int level, unsigned int/*tick*/)
{
  // This is more for debug purposes. I guess this condition should never pass.
  if(gpio != pin_a_ && gpio != pin_b_) {
    throw std::runtime_error(
      "Expected Encoder interrupt on pins " + std::to_string(pin_a_) + " or "
      + std::to_string(pin_b_) + ", however an interrupt was fired for pin "
      + std::to_string(gpio)
    );
  }

  // "Shift in time" the pin states
  a_past_ = a_current_;
  b_past_ = b_current_;
  // update the pint state that caused the interrupt
  (gpio == pin_a_ ? a_current_ : b_current_) = level;

  // Update the current step count
  direction_ = ENCODER_TABLE.at(encode(a_past_, b_past_, a_current_, b_current_));
  steps_ += direction_;

  // execute safety callbacks if needed
  if(steps_ <= lower_threshold_ && lower_cb_ != nullptr)
    lower_cb_();
  if(steps_ >= upper_threshold_ && upper_cb_ != nullptr)
    upper_cb_();
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
