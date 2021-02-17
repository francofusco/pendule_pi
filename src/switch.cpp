#include "pendule_pi/switch.hpp"
#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/debug.hpp"
#include <pigpio.h>


namespace pendule_pi {

Switch::Switch(
  int pin,
  bool normally_up,
  bool use_internal_pull_resistor
)
: pin_(pin)
, at_rest_(normally_up ? PI_HIGH : PI_LOW)
, debounce_us_(5000)
, last_tick_(0)
, at_rest_now_(true)
, has_triggered_(false)
, with_interrupts_(false)
, callback_(nullptr)
{
  PENDULE_PI_DBG("Creating Switch on pin " << pin_ << ". Normal state: " << (normally_up?"UP":"DOWN") << ". Internal resistor: " << (use_internal_pull_resistor?"YES":"NO"));
  // put the pin in input mode
  PiGPIO_RUN_VOID(gpioSetMode, pin, PI_INPUT);
  // (de)activate the pull-up/pull-down resistor as needed
  auto pull_mode = use_internal_pull_resistor ? (normally_up ? PI_PUD_UP : PI_PUD_DOWN) : PI_PUD_OFF;
  PiGPIO_RUN_VOID(gpioSetPullUpDown, pin, pull_mode);
  PENDULE_PI_DBG("Switch created");
}


Switch::~Switch() {
  PENDULE_PI_DBG("Destroying Switch on pin " << pin_);
  // set the pin in high impedance, just in case
  gpioSetPullUpDown(pin_, PI_PUD_OFF);
  gpioSetMode(pin_, PI_INPUT);
  disableInterrupts();
  PENDULE_PI_DBG("Switch destroyed successfully");
}


void Switch::enableInterrupts(
  std::function<void(void)> user_callback
)
{
  // set the callback to change the state of the Switch.
  gpioSetAlertFuncEx(pin_, Switch::onChangeStatic, this);
  callback_ = user_callback;
  with_interrupts_ = true;
}


void Switch::disableInterrupts() {
  gpioSetAlertFuncEx(pin_, nullptr, nullptr);
  callback_ = nullptr;
  with_interrupts_ = false;
}


bool Switch::atRest() const
{
  int val;
  PiGPIO_RUN(val, gpioRead, pin_);
  return val == at_rest_;
}


bool Switch::atRestCached() const
{
  if(!with_interrupts_)
    throw InterruptsAreDisabled();
  return at_rest_;
}


void Switch::onChange(
  int gpio,
  int level,
  unsigned int tick
)
{
  // This is more for debug purposes. I guess this condition should never pass.
  if(gpio != pin_) {
    throw std::runtime_error(
      "Expected Switch interrupt on pin " + std::to_string(pin_) + ", however"
      "an interrupt was fired for pin " + std::to_string(gpio)
    );
  }

  // Check if at least debounce_us_ microseconds have elapsed since the last
  // time we. served an interrupt. If not, just exit!
  if(tick - last_tick_ < debounce_us_) {
    return;
  }

  // Ok, let's serve the interrupt :)
  last_tick_ = tick;

  at_rest_now_ = (level == at_rest_);

  if(!at_rest_now_) {
    has_triggered_ = true;
    if(callback_)
      callback_();
  }
}


void Switch::onChangeStatic(
  int gpio,
  int level,
  unsigned int tick,
  void* THIS
)
{
  static_cast<Switch*>(THIS)->onChange(gpio, level, tick);
}



}
