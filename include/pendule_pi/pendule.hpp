#pragma once

#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/switch.hpp"
#include "pendule_pi/encoder.hpp"
#include "pendule_pi/motor.hpp"
#include <memory>


namespace pendule_pi {

/// Class that allows to control the Pendulum.
class Pendule {
public:
  /// Constructor, does not perform much...
  Pendule();

  /// Initialize the pendulum.
  void calibrate();

  /// Perform state estimation.
  /** @param dt time (in seconds) that elapsed since the last call to update().
    */
  void update(double dt);

  /// Get the current filtered position (in meters) of the base.
  inline const double& position() const { return position_; }
  /// Get the current filtered angle (in radians) of the pendulum.
  inline const double& angle() const { return angle_; }
  /// Get the current filtered velocity (in meters per second) of the base.
  inline const double& linearVelocity() const { return linvel_; }
  /// Get the current filtered velocity (in radians per second) of the pendulum.
  inline const double& angularVelocity() const { return angvel_; }

private:
  bool calibrated_; ///< Variable that is set to true once the pendulum calibration has been completed.
  // Hardware components
  std::unique_ptr<Motor> motor_; ///< Actuator to move the base of the pendulum.
  std::unique_ptr<Switch> left_switch_; ///< Left switch (should be near to the motor).
  std::unique_ptr<Switch> right_switch_; ///< Right switch (should be near to the encoder).
  std::unique_ptr<Encoder> position_encoder_; ///< Encoder to read the current position of the base.
  std::unique_ptr<Encoder> angle_encoder_; ///< Encoder to read the current angle of the pendulum.
  // State variables
  double position_; ///< Current position of the moving base.
  double angle_; ///< Current angle of the pendulum.
  double linvel_; ///< Current velocity of the moving base.
  double angvel_; ///< Current velocity of the pendulum.
  // Calibration values
  int min_position_steps_; ///< Encoder reading when the base is at the minimum position.
  int max_position_steps_; ///< Encoder reading when the base is at the maximum position.
  int mid_position_steps_; ///< Encoder reading when the base is at the middle position.
  double meters_per_step_; ///< Multiplicative factor to convert from encoder steps to meters.
  double radians_per_step_; ///< Multiplicative factor to convert from encoder steps to radians.
  double rest_angle_; ///< Position of the pendulum when it is at rest.

  /// Axuiliary method that converts steps into meters.
  inline double steps2meters(const int& steps) { return meters_per_step_*(steps-mid_position_steps_); }
  /// Axuiliary method that converts steps into radians.
  inline double steps2radians(const int& steps) { return radians_per_step_*steps-rest_angle_; }
};

}
