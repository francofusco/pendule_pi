#include "pendule_pi/pendule.hpp"
#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/debug.hpp"
#include <chrono>
#include <thread>
#include <cmath>

namespace pendule_pi {

Pendule::Pins::Pins()
: motor_pwm(24)
, motor_dir(16)
, left_switch(17)
, right_switch(18)
, position_encoder_a(20)
, position_encoder_b(21)
, angle_encoder_a(19)
, angle_encoder_b(26)
{
  // nothing else to do here
}


Pendule::Pendule(
  double meters_per_step,
  double radians_per_step,
  double rest_angle
)
: Pendule(
  meters_per_step,
  radians_per_step,
  rest_angle,
  Pins()
)
{
  // nothing else to do here
}


Pendule::Pendule(
  double meters_per_step,
  double radians_per_step,
  double rest_angle,
  const Pins& pins
)
: Pendule(
  meters_per_step,
  radians_per_step,
  rest_angle,
  std::make_unique<Motor>(pins.motor_pwm, pins.motor_dir),
  makeDefaultSwitch(pins.left_switch),
  makeDefaultSwitch(pins.right_switch),
  std::make_unique<Encoder>(pins.position_encoder_a, pins.position_encoder_b),
  std::make_unique<Encoder>(pins.angle_encoder_a, pins.angle_encoder_b)
)
{
  // nothing else to do here
}


Pendule::Pendule(
  double meters_per_step,
  double radians_per_step,
  double rest_angle,
  std::unique_ptr<Motor> motor,
  std::unique_ptr<Switch> left_switch,
  std::unique_ptr<Switch> right_switch,
  std::unique_ptr<Encoder> position_encoder,
  std::unique_ptr<Encoder> angle_encoder
)
: calibrated_(false)
, emergency_stopped_(false)
, meters_per_step_(meters_per_step/4)
, radians_per_step_(radians_per_step/4)
, rest_angle_(rest_angle)
, offset_up_(0)
, offset_down_(0)
, offset_static_(0)
, motor_(std::move(motor))
, left_switch_(std::move(left_switch))
, right_switch_(std::move(right_switch))
, position_encoder_(std::move(position_encoder))
, angle_encoder_(std::move(angle_encoder))
{
  PENDULE_PI_DBG("Creating Pendule object");
  // Default pin connections, in case we need to create any instance here
  Pins pins;
  if(motor_ == nullptr)
    motor_ = std::make_unique<Motor>(pins.motor_pwm, pins.motor_dir);
  if(left_switch_ == nullptr)
    left_switch_ = makeDefaultSwitch(pins.left_switch);
  if(right_switch_ == nullptr)
    right_switch_ = makeDefaultSwitch(pins.right_switch);
  if(position_encoder_ == nullptr)
    position_encoder_ = std::make_unique<Encoder>(pins.position_encoder_a, pins.position_encoder_b);
  if(angle_encoder_ == nullptr)
    angle_encoder_ = std::make_unique<Encoder>(pins.angle_encoder_a, pins.angle_encoder_b);
  PENDULE_PI_DBG("Pendule object created");
}


void Pendule::calibrate(
  double safety_margin_meters
)
{
  PENDULE_PI_DBG("Requested calibration via Pendule::calibrate(" << safety_margin_meters << ")");
  // Reset it no matter what!
  calibrated_ = false;
  // Ensure that we are not in emergency stop
  if(emergency_stopped_)
    throw CalibrationFailed("eStop has been called");
  // Make sure that we are not at an extremity
  if(!left_switch_->atRest())
    throw CalibrationFailed("left switch is not at rest");
  if(!right_switch_->atRest())
    throw CalibrationFailed("right switch is not at rest");
  // Reset all callbacks
  position_encoder_->removeSafetyCallbacks();
  angle_encoder_->removeSafetyCallbacks();
  left_switch_->disableInterrupts();
  right_switch_->disableInterrupts();
  // Calibrate the angle encoder. It simpli means that we want to ensure that
  // it is static and that its initial position is zero!
  const int angle_steps = angle_encoder_->steps();
  bool moving = false;
  angle_encoder_->setSafetyCallbacks(
    angle_steps-1,
    angle_steps+1,
    [&](){ moving=true; }
  );
  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  if(moving)
    throw CalibrationFailed("the pendulum is moving");
  if(angle_encoder_->steps() != 0)
    throw CalibrationFailed("the angle encoder is at a non-zero position");
  // Remove callbacks on the angle encoder
  angle_encoder_->removeSafetyCallbacks();
  // prepare some auxiliary callbacks used to reach the switches
  const int calibration_pwm = 40;
  bool wrong_switch = false;
  auto switch_cb = [&](){motor_->setPWM(0);};
  auto wrong_switch_cb = [&](){
    motor_->setPWM(0);
    wrong_switch = true;
  };
  // Go towards the right switch
  right_switch_->enableInterrupts(switch_cb);
  left_switch_->enableInterrupts(wrong_switch_cb);
  motor_->setPWM(calibration_pwm);
  while(motor_->getPWM() != 0) {
    if(wrong_switch) {
      throw CalibrationFailed(
        "hit the left switch while attempting to reach the right one"
      );
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  max_position_steps_ = position_encoder_->steps();
  // Release the right switch
  right_switch_->enableInterrupts(nullptr);
  motor_->setPWM(-calibration_pwm);
  while(!right_switch_->atRestCached())
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  motor_->setPWM(0);
  // Go towards the left switch
  right_switch_->enableInterrupts(wrong_switch_cb);
  left_switch_->enableInterrupts(switch_cb);
  motor_->setPWM(-calibration_pwm);
  while(motor_->getPWM() != 0) {
    if(wrong_switch) {
      throw CalibrationFailed(
        "hit the right switch while attempting to reach the left one"
      );
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  min_position_steps_ = position_encoder_->steps();
  // Release the left switch
  left_switch_->enableInterrupts(nullptr);
  motor_->setPWM(calibration_pwm);
  while(!left_switch_->atRestCached())
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  motor_->setPWM(0);
  // Check that min < max. If not, safety thresholds would be inverted!
  if(min_position_steps_ > max_position_steps_) {
    throw CalibrationFailed(
      "the minimum step position is greater than the maximum one. This likely "
      "means that the encoder 'reads backward'. Have you tried inverting the "
      "pahses? You could simply swap the pins in the constructor."
    );
  }
  mid_position_steps_ = (max_position_steps_ + min_position_steps_) / 2;
  // Go to the central position
  right_switch_->disableInterrupts();
  left_switch_->disableInterrupts();
  motor_->setPWM(calibration_pwm);
  while(position_encoder_->steps() < mid_position_steps_) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }
  motor_->setPWM(0);
  // Ok, we are in the central position!
  left_switch_->enableInterrupts([&](){ eStop("left switch hit"); });
  right_switch_->enableInterrupts([&](){ eStop("right switch hit"); });
  int safety_margin_steps = static_cast<int>(std::ceil(std::fabs(safety_margin_meters/meters_per_step_)));
  int soft_min = min_position_steps_ + safety_margin_steps;
  int soft_max = max_position_steps_ - safety_margin_steps;
  position_encoder_->setSafetyCallbacks(
    soft_min,
    soft_max,
    [&](){ eStop("soft minimum position limit reached"); },
    [&](){ eStop("soft maximum position limit reached"); }
  );
  soft_minmax_position_meters_ = std::fabs(steps2meters(soft_max));
  calibrated_ = true;
  PENDULE_PI_DBG("Calibration completed!");
  PENDULE_PI_DBG("min steps: " << min_position_steps_ << " (in meters: " << steps2meters(min_position_steps_) << ")");
  PENDULE_PI_DBG("max steps: " << max_position_steps_ << " (in meters: " << steps2meters(max_position_steps_) << ")");
  PENDULE_PI_DBG("middle steps: " << mid_position_steps_ << " (in meters: " << steps2meters(mid_position_steps_) << ")");
  PENDULE_PI_DBG("safety_margin_steps: " << safety_margin_steps << " (in meters: " << safety_margin_meters << ")");
  PENDULE_PI_DBG("soft_min: " << soft_min << " (in meters: " << steps2meters(soft_min) << ")");
  PENDULE_PI_DBG("soft_max: " << soft_max << " (in meters: " << steps2meters(soft_max) << ")");
}


void Pendule::update(
  double dt
)
{
  if(emergency_stopped_)
    throw std::runtime_error("Pendule::update(): eStop() has been called");
  if(!calibrated_)
    throw NotCalibrated("Pendule::update()");
  // Get raw encoder reading
  double new_position = steps2meters(position_encoder_->steps());
  double new_angle = steps2radians(angle_encoder_->steps());
  // State estimation: TODO!
  linvel_ = (new_position-position_) / dt;
  angvel_ = (new_angle-angle_) / dt;
  position_ = new_position;
  angle_ = new_angle;
}


bool Pendule::setCommand(
  int pwm
)
{
  if(emergency_stopped_)
    throw std::runtime_error("Pendule::setCommand(): eStop() has been called");
  if(!calibrated_)
    throw NotCalibrated("Pendule::setCommand()");
  // TODO: a more sophisticated processing of the signal!
  bool retval = true;
  if(pwm != 0) {
    if(pwm > 0)
      pwm += offset_up_;
    if(pwm < 0)
      pwm -= offset_down_;
    pwm += offset_static_;
    if(pwm > 255) {
      pwm = 255;
      retval = false;
    }
    if(pwm < -255) {
      pwm = -255;
      retval = false;
    }
  }
  motor_->setPWM(pwm);
  return retval;
}


const int& Pendule::minPositionSteps() const {
  if(!calibrated_)
    throw NotCalibrated("Pendule::minPositionSteps()");
  return min_position_steps_;
}


const int& Pendule::maxPositionSteps() const {
  if(!calibrated_)
    throw NotCalibrated("Pendule::maxPositionSteps()");
  return max_position_steps_;
}


const int& Pendule::midPositionSteps() const {
  if(!calibrated_)
    throw NotCalibrated("Pendule::midPositionSteps()");
  return mid_position_steps_;
}


const double& Pendule::softMinMaxPosition() const {
  if(!calibrated_)
    throw NotCalibrated("Pendule::softMinMaxPosition()");
  return soft_minmax_position_meters_;
}


void Pendule::setPwmOffsets(
  int offset_down,
  int offset_up
)
{
  offset_up_ = std::max(0, offset_up);
  offset_down_ = std::max(0, offset_down);
}


void Pendule::setPwmOffsets(
  int offset_static
)
{
  offset_up_ = 0;
  offset_down_ = 0;
  offset_static_ = offset_static;
}


void Pendule::setPwmOffsets(
  int offset_static,
  int offset_down,
  int offset_up
)
{
  offset_static_ = offset_static;
  offset_up_ = std::max(0, offset_up);
  offset_down_ = std::max(0, offset_down);
}


void Pendule::eStop(
  const std::string& why
)
{
  motor_->setPWM(0);
  emergency_stopped_ = true;
  throw EmergencyStop(why);
}


}
