#pragma once
#include <pendule_pi/switch.hpp>
#include <pendule_pi/encoder.hpp>
#include <pendule_pi/motor.hpp>
#include <pendule_pi/pendule.hpp>
#include <memory>


namespace pendule_pi {

/// Class that allows to control the Pendulum.
class Pendule {
public:
  /// Exception class to be thrown when the pendulum has not been calibrated.
  class NotCalibrated : public std::runtime_error {
  public:
    /// Fills the error message.
    NotCalibrated() : std::runtime_error("Pendule was not calibrated yet!") {}
    /// Fills the error message.
    /** @param src the "source" of this exception, *e.g.*, the method that was
      *   called before performing the calibration.
      */
    NotCalibrated(const std::string& src) : std::runtime_error(src + ": Pendule was not calibrated yet!") {}
  };

  /// Exception class to be thrown when the calibration fails for any reason.
  class CalibrationFailed : public std::runtime_error {
  public:
    /// Fills the error message.
    /** @param why reason for the failure.
      */
    CalibrationFailed(const std::string& why) : std::runtime_error("Pendulum calibration failed! Reason: " + why) {}
  };

  /// Exception class to be thrown when emergency stop is requested.
  class EmergencyStop : public std::runtime_error {
  public:
    /// Fills the error message.
    /** @param why reason to stop.
      */
    EmergencyStop(const std::string& why) : std::runtime_error("Pendulum::eStop has been called! Reason: " + why) {}
  };

  /// Creates the pendulum, using the given components.
  /** @param meters_per_step distance covered by the base when the associated
    *   encoder registers a single step. As an example, say that the rotatory
    *   encoder features 100 steps per revolution and that it is connected to
    *   the base using a belt and a pully of diameter 10mm. Due to the pully,
    *   the transmission factor is thus
    *   \f$ 2 \pi 10 \text{mm} / \text{rotation} \f$.
    *   Since the encoder features 100 steps per rotation, the final factor is:
    *   \f$ \frac{2 \pi 10}{100} \text{mm} / \text{step}
    *       = 2 \pi 10^{-4} \text{m} / \text{step} \f$.
    * @param radians_per_step angle spanned by the pendulum when the associated
    *   encoder registers a single step. As an example, say that the rotatory
    *   encoder features 100 steps per revolution and that it is directly
    *   connected to the pendulum. The transmission factor is thus
    *   \f$ \frac{2 \pi}{100} \text{rad} / \text{step} \f$.
    * @param safety_margin_meters distance from the switches at which the
    *   pendulum should request an emergency stop. This margin allows to add
    *   a software-based level of safety, since the system should shutdown
    *   before actually experiencing collisions.
    * @param rest_angle value, in radians, that should be returned by angle()
    *   when the pendulum points downward due to gravity.
    * @param motor the Motor that allows to actuate the base. It should be
    *   wired in such a way that a positive command moves the base to the right.
    * @param left_switch the Switch that should be reached when the base moves
    *   in the negative direction.
    * @param right_switch the Switch that should be reached when the base moves
    *   in the positive direction.
    * @param position_encoder rotary encoder used to measure the position of
    *   the base.
    * @param angle_encoder rotary encoder used to measure the angle of the
    *   pendulum.
    * @note As mentioned in the Encoder class, the number of steps per
    *   revolution that an Encoder instance can register is actually 4 times
    *   the one declared by the constructor. As an example, with a 600 steps
    *   encoder, the corresponding Encoder object will count 2400 steps per
    *   full rotation. This factor is added internally here, and therefore
    *   **you should not take it into account when calculating the parameters
    *   meters_per_step and radians_per_step**.
    * @warning Ownership of the objects contained in the pointers passed to this
    *   constructor is claimed. You should not keep any copy of these objects,
    *   and in particular you should let this class manage their memory.
    *   Any of the unique pointers passed to this constructor can alternatively
    *   be `nullptr`. In this case, default instances will be generated and
    *   managed internally.
    */
  Pendule(
    double meters_per_step,
    double radians_per_step,
    double safety_margin_meters,
    double rest_angle,
    std::unique_ptr<Motor> motor,
    std::unique_ptr<Switch> left_switch,
    std::unique_ptr<Switch> right_switch,
    std::unique_ptr<Encoder> position_encoder,
    std::unique_ptr<Encoder> angle_encoder
  );

  // Prevent the user from making copies of a Pendule.
  Pendule(const Pendule&) = delete;
  Pendule& operator=(const Pendule&) = delete;

  /// Initialize the pendulum.
  void calibrate(int calibration_pwm);

  /// Tells if the pendulum has been calibrated successfully.
  const inline bool& isCalibrated() const { return calibrated_; }

  /// Perform state estimation.
  /** @param dt time (in seconds) that elapsed since the last call to update().
    */
  void update(double dt);

  /// Forwards the command to the actuator.
  /** Applies the given PWM to the motor.
    * @param pwm the desired command.
    * @return false if the command exceeded the maximum/minimum values and thus
    *   had to be saturated. Note that the saturation is applied after adding
    *   the offsets specified via setPwmOffsets().
    */
  bool setCommand(int pwm);

  /// Get the current filtered position (in meters) of the base.
  inline const double& position() const { return position_; }
  /// Get the current filtered angle (in radians) of the pendulum.
  inline const double& angle() const { return angle_; }
  /// Get the current filtered velocity (in meters per second) of the base.
  inline const double& linearVelocity() const { return linvel_; }
  /// Get the current filtered velocity (in radians per second) of the pendulum.
  inline const double& angularVelocity() const { return angvel_; }

  /// Allow to access min_position_steps_.
  /** If the pendulum has not been calibrated, this method will throw an
    * exception.
    * @return the value of min_position_steps_.
    */
  const int& minPositionSteps() const;

  /// Allow to access max_position_steps_.
  /** If the pendulum has not been calibrated, this method will throw an
    * exception.
    * @return the value of max_position_steps_.
    */
  const int& maxPositionSteps() const;

  /// Allow to access mid_position_steps_.
  /** If the pendulum has not been calibrated, this method will throw an
    * exception.
    * @return the value of mid_position_steps_.
    */
  const int& midPositionSteps() const;

  /// Read the value of the soft position limits (in meters).
  const double& softMinMaxPosition() const;

  /// Set offsets to compensate for friction.
  /** This function sets internal offsets that should be applied to the pwm
    * passed to setCommand(). This should allow to compensate for static
    * friction. Different offsets are to be given depending on the direction.
    * Of course, a null pwm signal will not be altered.
    * @param offset_down value to be subtracted from a negative pwm passed to
    *   setCommand(pwm). If the desired pwm is negative, then the actual value
    *   sent to the motor will be pwm-offset_down.
    * @param offset_up value to be added to a positive pwm passed to
    *   setCommand(pwm). If the desired pwm is positive, then the actual value
    *   sent to the motor will be pwm+offset_up.
    * @note This reduces the range of "feasible" pwm from `[-255,255]` to
    *   [-255+offset_down,255-offset_up].
    * @warning Both input parameters should be non-negative. If you provide a
    *   negative value, 0 will be used instead.
    */
  void setPwmOffsets(
    int offset_down,
    int offset_up
  );

  /// Emergency stop.
  void eStop(const std::string& why);

private:
  bool calibrated_; ///< Variable that is set to true once the pendulum calibration has been completed.
  bool emergency_stopped_; ///< Variable that is set to true when the pendulum has to stop.
  // State variables
  double position_; ///< Current position of the moving base.
  double angle_; ///< Current angle of the pendulum.
  double linvel_; ///< Current velocity of the moving base.
  double angvel_; ///< Current velocity of the pendulum.
  // Calibration values
  int min_position_steps_; ///< Encoder reading when the base is at the minimum position.
  int max_position_steps_; ///< Encoder reading when the base is at the maximum position.
  int mid_position_steps_; ///< Encoder reading when the base is at the middle position.
  double safety_margin_meters_; ///< Safety distance from the switches at which emergency stop is requested.
  double soft_minmax_position_meters_; ///< Position (in meters) at which emergency stop is requested.
  const double meters_per_step_; ///< Multiplicative factor to convert from encoder steps to meters.
  const double radians_per_step_; ///< Multiplicative factor to convert from encoder steps to radians.
  const double rest_angle_; ///< Position of the pendulum when it is at rest.
  int offset_up_; ///< Offset to be applied to positive pwm commands.
  int offset_down_; ///< Offset to be applied to negative pwm commands.
  int offset_static_; ///< Static offset to be applied to the command.
  // Hardware components
  std::unique_ptr<Motor> motor_; ///< Actuator to move the base of the pendulum.
  std::unique_ptr<Switch> left_switch_; ///< Left switch (should be near to the motor).
  std::unique_ptr<Switch> right_switch_; ///< Right switch (should be near to the encoder).
  std::unique_ptr<Encoder> position_encoder_; ///< Encoder to read the current position of the base.
  std::unique_ptr<Encoder> angle_encoder_; ///< Encoder to read the current angle of the pendulum.

  /// Axuiliary method that converts steps into meters.
  inline double steps2meters(const int& steps) { return meters_per_step_*(steps-mid_position_steps_); }
  /// Axuiliary method that converts steps into radians.
  inline double steps2radians(const int& steps) { return radians_per_step_*steps-rest_angle_; }
};

}
