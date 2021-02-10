/** @file motor.hpp
  * @brief Header file for the Motor class.
  */
#pragma once

namespace pendule_pi {

/// Class that handles a simple DC motor.
class Motor {
public:
  /// Constructor.
  /** @param pwm_pin Pin used to control the force of the motor via PWM.
    * @param direction_pin Pin used to control the direction of the motor.
    */
  Motor(
    int pwm_pin,
    int direction_pin
  );

  /// Destructor.
  virtual ~Motor();

  /// Sets the pwm of the motor.
  /** @param pwm the (signed) pwm, between -255 and 255 (bith included).
    */
  void setPWM(
    int pwm
  );

  /// Get the current PWM value.
  inline const int& getPWM() const { return pwm_; }

private:
  int pwm_pin_; ///< Pin used to control the force of the motor via PWM.
  int dir_pin_; ///< Pin used to control the direction of the motor.
  int pwm_; ///< Current PWM applied to pwm_pin_.
};

}
