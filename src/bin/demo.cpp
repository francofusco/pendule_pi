#include <pendule_pi/pigpio.hpp>
#include <pendule_pi/pendule.hpp>
#include <pendule_pi/joystick.hpp>
#include <iir_filters/common_filters.hpp>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cmath>


// Simple function that returns zero if |val|<zero and val otherwise.
int robustZero(int val, int zero) {
  if(-zero < val && val < zero)
    return 0;
  else
    return val;
}

int clip(int val, int zero) {
  if(val > zero)
    return val - zero;
  else if(val < -zero)
    return val + zero;
  else
    return 0;
}

// Simple sign function
inline double sign(double x) {
  return x>=0 ? 1.0 : -1.0;
}

// Modular shift in the unit circle
double shiftInCircle(double x)
{
  double v = std::fmod(x, 2*M_PI);
  if(v >= M_PI)
    v -= 2*M_PI;
  if(v < -M_PI)
    v += 2*M_PI;
  return v;
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

  // TODO: allow to change the axis!
  const int AXIS_PWM = 0;
  const int BTN_EXIT = 1;
  const int BTN_SWITCH = 0;
  const int BTN_PERMISSION = 2;

  try {
    // Let the token manage the pigpio library!
    pigpio::ActivationToken token;
    // Create the joystick device to be used for controlling the demo.
    pp::Joystick joy;
    // Create the pendulum instance and perform the calibration.
    //pp::Pendule pendule(0.846/21200, 2*M_PI/1000, 0.0);
    const double ANGLE_OFFSET = 0.0;
    const double POSITION_RATIO_I3S = 0.846/21200;
    const double POSITION_RATIO_MIA = POSITION_RATIO_I3S*1000.0/600.0;
    pp::Pendule pendule(POSITION_RATIO_MIA, 2*M_PI/600, ANGLE_OFFSET);
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(0.05);
    pendule.setPwmOffsets(2, 15, 15);
    std::cout << "Calibration completed!" << std::endl;
    // Define soft limits for the pendulum.
    const double MAX_POSITION = pendule.softMinMaxPosition() - 0.1;
    // Create the timer used for enforcing a stable control rate.
    const int SLEEP_MS = 20;
    const double SLEEP_SEC = SLEEP_MS/1000.0;
    pigpio::Rate rate(SLEEP_MS*1000);
    // Create the timer used to print some information on the screen
    const int COUT_MS = 100;
    pigpio::Timer cout_timer(COUT_MS*1000, true);
    // Timer used to update the joystick at a slightly lower rate
    const int JOY_MS = 100;
    pigpio::Timer joy_timer(JOY_MS*1000, true);
    // Used to allow the user to detect when buttons are released or pressed
    CachedButton btn_switch;
    // Variables used to perform control and filtering
    int pwm = 0;
    ControlMode control_mode(ControlMode::Manual);
    // Filters
    double Fs = 1.0/SLEEP_SEC; // sampling frequency
    double Fc = Fs/4; // cutoff frequency
    auto filter_position = iir_filters::butterworth<double,double>(4, Fc, Fs);
    auto filter_angle = iir_filters::butterworth<double,double>(4, Fc, Fs);
    auto filter_linvel = iir_filters::butterworth<double,double>(4, Fc, Fs);
    auto filter_angvel = iir_filters::butterworth<double,double>(4, Fc, Fs);
    filter_position.initInput(pendule.position());
    filter_position.initOutput(pendule.position());
    filter_angle.initInput(pendule.angle());
    filter_angle.initOutput(pendule.angle());
    filter_linvel.initInput(0.0);
    filter_linvel.initOutput(0.0);
    filter_angvel.initInput(0.0);
    filter_angvel.initOutput(0.0);
    auto filter_target_position = iir_filters::butterworth<double,double>(2, Fs/20, Fs);
    // Target position
    double target_pos = 0;
    const double target_angle = M_PI;
    // control gains for the swingup phase
    const double kswing = 100.0;
    const double ksx = 0.0;
    // control gains for the lqr phase
    //const double kp = -127.45880662905581;
    //const double kpd = -822.638944546691;
    //const double kt = 2234.654627319883;
    //const double ktd = 437.1177135919267;
    const double kp = -135.35;
    const double kpd = -847.89;
    const double kt = 2621.19;
    const double ktd = 597.93;
    // process variables
    double filtered_position;
    double filtered_angle;
    double filtered_linvel;
    double filtered_angvel;
    double e_pos, e_theta, e_linvel, e_angvel;
    const double ANGLE_ERROR_THRESHOLD = 0.15;

    // Some logging information
    std::cout << "  MODE     POSITION        ANGLE       LIN.VEL.      ANG.VEL.     PWM" << std::endl;
                //SWINGUP  +000000.0000  +000000.0000  +000000.0000  +000000.0000  +000
    std::cout << std::setfill('0') << std::internal << std::showpos << std::fixed << std::setprecision(4);
    // sleep a little bit before starting with the main loop
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // Main loop!
    while(true) {
      // Sleep and update the state of the pendulum
      rate.sleep();
      pendule.update(SLEEP_SEC);
      // perform state filtering
      filtered_position = filter_position.filter(pendule.position());
      filtered_angle = filter_angle.filter(pendule.angle());
      filtered_linvel = filter_linvel.filter(pendule.linearVelocity());
      filtered_angvel = filter_angvel.filter(pendule.angularVelocity());

      // update the joystick when needed
      if(joy_timer.expired()) {
        joy.update();
        // should we exit?
        if(joy.button(BTN_EXIT)) {
          std::cout << std::endl << "Qutting!" << std::endl;
          throw pigpio::ActivationToken::PleaseStop();
        }
        // should we switch control mode?
        btn_switch.update(joy.button(BTN_SWITCH));
        if(btn_switch.pressed()) {
          control_mode = next(control_mode);
        }
      }

      if(control_mode == ControlMode::Manual) {
        int ref = joy.axis(AXIS_PWM);
        ref = robustZero(ref, 2500);
        pwm = static_cast<int>(ref*250./32768.);
      }
      else if(control_mode == ControlMode::LQR || control_mode == ControlMode::Swingup) {
        // Errors (into [-pi,+pi] for the angle)
        target_pos = filter_target_position.filter( clip(joy.axis(AXIS_PWM), 2500) * 0.2 / 32768.);
        e_pos = filtered_position - target_pos;
        e_theta = shiftInCircle(filtered_angle - target_angle);
        e_linvel = filtered_linvel;
        e_angvel = filtered_angvel;

        if(std::fabs(e_theta) > ANGLE_ERROR_THRESHOLD) {
          // not in LQR range: the command depends on the current mode
          if(control_mode == ControlMode::LQR) {
            pwm = 0;
          }
          else {
            // Swingup
            pwm = static_cast<int>(
              + kswing * (1-std::cos(e_theta)) * sign(filtered_angvel * std::cos(e_theta))
              - ksx * filtered_position
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
        std::cout << std::endl << "ERROR: " << to_string(control_mode) << " mode not implemented yet" << std::endl;
        throw pigpio::ActivationToken::PleaseStop();
      }

      // Enforce soft safety limits, then send the command.
      if(pendule.position() > MAX_POSITION && pwm > 0)
        pwm = 0;
      else if(pendule.position() < -MAX_POSITION && pwm < 0)
        pwm = 0;
      pendule.setCommand(pwm);

      // print on screen once in a while
      if(cout_timer.expired()) {
        std::cout << "\r";
        std::cout << to_string(control_mode);
        std::cout << "  " << std::setw(12) << pendule.position();
        std::cout << "  " << std::setw(12) << pendule.angle();
        std::cout << "  " << std::setw(12) << pendule.linearVelocity();
        std::cout << "  " << std::setw(12) << pendule.angularVelocity();
        std::cout << "  " << std::setw(4) << pwm;
        std::cout << "   " << std::flush;
      }
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
