#pragma once

#include <memory>
#include <zmqpp/zmqpp.hpp>

namespace pendule_pi {

/// Bridge to the low-level interface.
class PenduleCpp {
public:
  static auto constexpr DEFAULT_HOST = "localhost";
  static auto constexpr DEFAULT_STATE_PORT = "10001";
  static auto constexpr DEFAULT_COMMAND_PORT = "10002";

  /// Named option to be passed to PenduleCpp::readState.
  /** @see PenduleCpp::readState */
  static auto constexpr BLOCKING = true;
  /// Named option to be passed to PenduleCpp::readState.
  /** @see PenduleCpp::readState */
  static auto constexpr NON_BLOCKING = false;

  /// Connects to a socket using some default parameters.
  PenduleCpp(int wait=-1);

  /// Constructor, initializes socket connections.
  /** Connections are established at `tcp://[host]:[port]`. The
    * @param host string that tells the host for the socket.
    * @param state_port port of the socket that is used to read the current
    *   state of the pendulum.
    * @param command_port port of the socket that is used to send commands to
    *   the low-level interface.
    * @param wait time to wait for the interface, in seconds. If less than or
    *   equal to zero, wait indefinitely. If positive, the constructor will check
    *   if a message is available every second, until wait seconds have elapsed.
    *   If no message is received within the allotted time, an exception will be
    *   thrown.
    */
  PenduleCpp(
    const std::string& host,
    const std::string& state_port,
    const std::string& command_port,
    int wait = -1
  );

  /// Deallocates the memory for the sockets.
  ~PenduleCpp();

  /// Allows to access the current time of the pendulum.
  inline const double& time() const { return time_; }
  /// Allows to access the current position of the pendulum.
  inline const double& position() const { return position_; }
  /// Allows to access the current angle of the pendulum.
  inline const double& angle() const { return angle_; }
  /// Allows to access the current linear velocity of the pendulum.
  inline const double& linvel() const { return linvel_; }
  /// Allows to access the current angular velocity of the pendulum.
  inline const double& angvel() const { return angvel_; }

  /// Tries to read the state of the pendulum from the interface.
  /** @param blocking if `true`, do not exit until a message has been received
    *   from the socket. If `false`, exit immediately if no messages are
    *   received.
    * @return `true` if a message has been received and processed. Note that if
    *   the call is blocking, the function should always return `true`.
    * @note To make the code more readable, the members `BLOCKING` and
    *   `NON_BLOCKING` have been defined. You are encouraged to call this
    *   method as, *e.g*.:
    *   @code{.c++}
    *   PenduleCpp pendulum;
    *   pendulum.readState(PenduleCpp.BLOCKING);
    *   pendulum.readState(PenduleCpp.NON_BLOCKING);
    *   @endcode
    */
  bool readState(bool blocking);

  /// Send a PWM command to the low-level interface.
  /** @param pwm the PWM signal to be sent. It should be an integer between
    *   -255 and 255.
    */
  void sendCommand(int pwm);

private:
  double time_{0}; ///< Current time of the pendulum.
  double position_{0}; ///< Current position of the pendulum.
  double angle_{0}; ///< Current angle of the pendulum.
  double linvel_{0}; ///< Current linear velocity of the pendulum.
  double angvel_{0}; ///< Current angular velocity of the pendulum.

  std::unique_ptr<zmqpp::context> context_; ///< ZeroMQ context used to create TCP connections.
  std::unique_ptr<zmqpp::socket> state_sub_; ///< Socket to read the current state of the pendulum.
  std::unique_ptr<zmqpp::socket> command_pub_; ///< Socket to send commands to the low-level interface.

};

} // namespace pendule_pi
