#include <pendule_pi/pendule_cpp_client.hpp>
#include <pendule_pi/pigpio.hpp>
#include <pendule_pi/joystick.hpp>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include "utils.hpp"


bool ok{true};

void sigintHandler(int signo) {
  ok = false;
}

enum class ControlMode {
  Manual,
  LQR,
  Swingup
};


std::string to_string(const ControlMode& mode) {
  if(mode == ControlMode::Manual) {
    return std::string(" manual");
  }
  else if(mode == ControlMode::LQR) {
    return std::string("  lqr  ");
  }
  else if(mode == ControlMode::Swingup) {
    return std::string("swingup");
  }
  return std::string("ERROR");
}


ControlMode next(const ControlMode& mode) {
  if(mode == ControlMode::Manual)
    return ControlMode::LQR;
  else if(mode == ControlMode::LQR)
    return ControlMode::Swingup;
  else if(mode == ControlMode::Swingup)
    return ControlMode::Manual;

  return ControlMode::Manual;
}


class CachedButton {
public:
  inline void update(int state) { past_ = current_; current_ = state; }
  inline bool down() const { return current_; }
  inline bool switched() const { return current_ != past_; }
  inline bool pressed() const { return current_ && !past_; }
  inline bool released() const { return !current_ && past_; }
private:
  bool current_{false};
  bool past_{false};
};


int main(int argc, char** argv) {
  namespace pp = pendule_pi;



  if(argc >= 2 && (argv[1] == std::string("help") || argv[1] == std::string("-h") || argv[1] == std::string("--help"))) {
    YAML::Emitter out;
    out << YAML::BeginMap;
      out << YAML::Key << "sockets";
      out << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "host";
        out << YAML::Value << "localhost";
        out << YAML::Key << "state_port";
        out << YAML::Value << "10001";
        out << YAML::Key << "command_port";
        out << YAML::Value << "10002";
      out << YAML::EndMap;
      out << YAML::Key << "gains";
      out << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "position";
        out << YAML::Value << -127.45880662905581;
        out << YAML::Key << "linvel";
        out << YAML::Value << -822.638944546691;
        out << YAML::Key << "angle";
        out << YAML::Value << 2234.654627319883;
        out << YAML::Key << "angvel";
        out << YAML::Value << 437.1177135919267;
      out << YAML::EndMap;
      out << YAML::Key << "swingup";
      out << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "k";
        out << YAML::Value << 70;
        out << YAML::Key << "kx";
        out << YAML::Value << 0;
      out << YAML::EndMap;
      out << YAML::Key << "joystick";
      out << YAML::Value << YAML::BeginMap;
        out << YAML::Key << "device";
        out << YAML::Value << "/dev/input/js0";
        out << YAML::Key << "axis_pwm";
        out << YAML::Value << 0;
        out << YAML::Key << "btn_exit";
        out << YAML::Value << 0;
        out << YAML::Key << "btn_switch";
        out << YAML::Value << 1;
        out << YAML::Key << "btn_permission";
        out << YAML::Value << 4;
      out << YAML::EndMap;
    out << YAML::EndMap;
    std::cout << "Usage: " << argv[0] << " config.yaml" << std::endl;
    std::cout << "Example of configuration file to be given:" << std::endl;
    std::cout << out.c_str() << std::endl;
    return EXIT_SUCCESS;
  }

  // Load the configuration for this demo.
  YAML::Node yaml;
  if(argc > 1)
    yaml = YAML::LoadFile(argv[1]);
  // Conect to the low-level interface.
  pp::PenduleCppClient pendulum(
    yaml["socket"]["host"].as<std::string>("localhost"),
    yaml["socket"]["state_port"].as<std::string>("10001"),
    yaml["socket"]["command_port"].as<std::string>("10002"),
    5
  );
  // Create the joystick device to be used for controlling the demo.
  pp::Joystick joy(yaml["joystick"]["device"].as<std::string>("/dev/input/js0"));
  // Joystick buttons.
  const int AXIS_PWM = yaml["axis_pwm"].as<int>(0);
  const int BTN_EXIT = yaml["btn_exit"].as<int>(0);
  const int BTN_SWITCH = yaml["btn_switch"].as<int>(1);
  const int BTN_PERMISSION = yaml["btn_permission"].as<int>(4);
  // Counters used to update the joystick at a slightly lower rate.
  unsigned int JOY_UPDATE_MAX = 5;
  unsigned int joy_update_counter = 0;
  // Used to allow the user to detect when buttons are released or pressed.
  CachedButton btn_switch;
  // Variables used to perform control.
  int pwm = 0;
  ControlMode control_mode(ControlMode::Manual);
  double target_pos = 0;
  double e_pos, e_theta, e_linvel, e_angvel;
  const double target_angle = M_PI;
  const double ANGLE_ERROR_THRESHOLD = 0.25;
  // control gains for the swingup phase
  const double kswing = yaml["swingup"]["k"].as<double>(70.0);
  const double ksx = yaml["swingup"]["kx"].as<double>(0.0);
  // control gains for the lqr phase
  const double kp = yaml["gains"]["position"].as<double>(-127.45880662905581);
  const double kpd = yaml["gains"]["linvel"].as<double>(-822.638944546691);
  const double kt = yaml["gains"]["angle"].as<double>(2234.654627319883);
  const double ktd = yaml["gains"]["angvel"].as<double>(437.1177135919267);

  // Custom signal handler.
  std::signal(SIGINT, sigintHandler);

  std::cout << "  MODE" << std::endl;
  std::cout << to_string(control_mode) << std::flush;

  // Main loop!
  while(ok) {

    // Retrieve the current state of the pendulum.
    pendulum.readState(pendulum.BLOCKING);

    // Do control!
    if(control_mode == ControlMode::Manual) {
      // Get the command from the joystick.
      int ref = joy.axis(AXIS_PWM);
      ref = robustZero(ref, 2500);
      pwm = static_cast<int>(ref*250./32768.);
    }
    else if(control_mode == ControlMode::LQR || control_mode == ControlMode::Swingup) {
      // Errors (into [-pi,+pi] for the angle)
      target_pos = clip(joy.axis(AXIS_PWM), 2500) * 0.2 / 32768.;
      e_pos = pendulum.position() - target_pos;
      e_theta = shiftInCircle(pendulum.angle() - target_angle);
      e_linvel = pendulum.linvel();
      e_angvel = pendulum.angvel();

      if(std::fabs(e_theta) > ANGLE_ERROR_THRESHOLD) {
        // Not in LQR range: the command depends on the current mode.
        if(control_mode == ControlMode::LQR) {
          pwm = 0;
        }
        else {
          // Swingup
          auto ks = kswing * ( 0.1 + 0.9 * 0.5 * (1-std::cos(e_theta)) );
          pwm = static_cast<int>(
            + ks * (1-std::cos(e_theta)) * sign(pendulum.angvel() * std::cos(e_theta))
            - ksx * pendulum.position()
          );
        }
      }
      else {
        // in LQR range: do it!
        pwm = static_cast<int>(
          - kp * e_pos
          - kpd * e_linvel
          - kt * e_theta
          - ktd * e_angvel
        );
      }

      // override command if the permission button is not down ;)
      if(!joy.button(BTN_PERMISSION)) {
        pwm = 0;
      }
    }
    else {
      std::cout << std::endl << "ERROR: " << to_string(control_mode) << " mode not implemented yet";
      break;
    }

    pendulum.sendCommand(pwm);

    // update the joystick when needed
    joy_update_counter = (joy_update_counter + 1) % JOY_UPDATE_MAX;
    if(joy_update_counter == 0) {
      joy.update();
      // should we exit?
      if(joy.button(BTN_EXIT)) {
        std::cout << std::endl << "Qutting!";
        break;
      }
      // should we switch control mode?
      btn_switch.update(joy.button(BTN_SWITCH));
      if(btn_switch.pressed()) {
        control_mode = next(control_mode);
        std::cout << "\r" << to_string(control_mode) << std::flush;
      }
    }
  }

  pendulum.sendCommand(0);
  std::cout << std::endl;

  return EXIT_SUCCESS;
}
