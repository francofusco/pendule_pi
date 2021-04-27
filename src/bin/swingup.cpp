#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/pendule.hpp"
#include <iir_filters/common_filters.hpp>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cmath>



inline double sign(double x) {
  return x>=0 ? 1.0 : -1.0;
}

double shiftInCircle(double x)
{
  double v = std::fmod(x, 2*M_PI);
  if(v >= M_PI)
    v -= 2*M_PI;
  if(v < -M_PI)
    v += 2*M_PI;
  return v;
}


int main() {
  namespace pp = pendule_pi;

  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the encoder
    const double ANGLE_OFFSET = -0.0026;
    const double POSITION_RATIO_I3S = 0.846/21200;
    const double POSITION_RATIO_MIA = POSITION_RATIO_I3S*1000.0/600.0;
    pp::Pendule pendule(POSITION_RATIO_MIA, 2*M_PI/600, ANGLE_OFFSET);
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(0.05);
    std::cout << "Calibration completed!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    double MAX_POSITION = pendule.softMinMaxPosition() - 0.05;
    // pendule.setPwmOffsets(20, 30);
    // pendule.setPwmOffsets(2);
    pendule.setPwmOffsets(2, 15, 15);
    const int SLEEP_MS = 10;
    const double SLEEP_SEC = SLEEP_MS/1000.0;
    pigpio::Rate rate(SLEEP_MS*1000);
    // std::cout << "    POSITION        ANGLE       LIN.VEL.      ANG.VEL.     PWM" << std::endl;
    //             //  +000000.0000  +000000.0000  +000000.0000  +000000.0000  +000
    // std::cout << std::setfill('0') << std::internal << std::showpos << std::fixed << std::setprecision(4);

    // Filtering
    double Fs = 1.0/SLEEP_SEC; // sampling frequency
    double Fc = 0.15*Fs; // cutoff frequency
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
    double target_pos = 0;

    const double kswing = 90.0;
    const double ksx = 0.0;

    const double kp = -135.35;
    const double kpd = -847.89;
    const double kt = 2621.19;
    const double ktd = 597.93;

    double filtered_position;
    double filtered_angle;
    double filtered_linvel;
    double filtered_angvel;
    double e_theta=0;
    int pwm = 0;

    while(true) {
      // state estimation
      rate.sleep();
      pendule.update(SLEEP_SEC);
      // filter the state
      filtered_position = filter_position.filter(pendule.position());
      filtered_angle = filter_angle.filter(pendule.angle());
      filtered_linvel = filter_linvel.filter(pendule.linearVelocity());
      filtered_angvel = filter_angvel.filter(pendule.angularVelocity());
      // Error into [-pi,+pi]
      e_theta = shiftInCircle(filtered_angle - M_PI);

      if(std::fabs(e_theta) > 0.2) {
        pwm = static_cast<int>(
          + kswing * (1-std::cos(e_theta)) * sign(filtered_angvel * std::cos(e_theta))
          - ksx * filtered_position
        );
      }
      else {
        pwm = static_cast<int>(
          std::round(
            - kp * filtered_position
            - kpd * filtered_linvel
            - kt * e_theta
            - ktd * filtered_angvel
          )
        );
      }
      if(pendule.position() > MAX_POSITION && pwm > 0)
        pwm = 0;
      else if(pendule.position() < -MAX_POSITION && pwm < 0)
        pwm = 0;
      pendule.setCommand(pwm);
      // print on screen
      // std::cout << "\r";
      // std::cout << "  " << std::setw(12) << pendule.position();
      // // std::cout << "  " << std::setw(12) << pendule.angle();
      // std::cout << "  " << std::setw(12) << e_theta;
      // std::cout << "  " << std::setw(12) << pendule.linearVelocity();
      // std::cout << "  " << std::setw(12) << pendule.angularVelocity();
      // std::cout << "  " << std::setw(4) << pwm;
      // std::cout << "   " << std::flush;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
