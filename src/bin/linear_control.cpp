#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/pendule.hpp"
#include "pendule_pi/joystick.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cmath>


int main() {
  namespace pp = pendule_pi;

  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the encoder
    pp::Joystick joy;
    pp::Pendule pendule(0.846/21200, 2*M_PI/1000, 0.0);
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(0.05);
    std::cout << "Calibration completed!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    double MAX_POSITION = pendule.softMinMaxPosition() - 0.1;
    // pendule.setPwmOffsets(20, 30);
    const int SLEEP_MS = 20;
    const double SLEEP_SEC = SLEEP_MS/1000.0;
    pigpio::Rate rate(SLEEP_MS*1000);
    std::cout << "    POSITION        ANGLE       LIN.VEL.      ANG.VEL.     PWM" << std::endl;
                //  +000000.0000  +000000.0000  +000000.0000  +000000.0000  +000
    std::cout << std::setfill('0') << std::internal << std::showpos << std::fixed << std::setprecision(4);

    const double kp = -31.46308258;
    const double kpd = -817.63240318;
    const double kt = 2230.15105779;
    const double ktd = 436.32952145;
    double e_theta=0;
    int pwm = 0;

    while(true) {
      // state estimation
      rate.sleep();
      pendule.update(SLEEP_SEC);
      // Compute command
      e_theta = pendule.angle()-M_PI;
      if(std::fabs(e_theta) > 0.1) {
        pwm = 0;
      }
      else {
        pwm = static_cast<int>(
          - kp * pendule.position()
          - kpd * pendule.linearVelocity()
          - kt * e_theta
          - ktd * pendule.angularVelocity()
        );
      }
      if(pendule.position() > MAX_POSITION && pwm > 0)
        pwm = 0;
      else if(pendule.position() < -MAX_POSITION && pwm < 0)
        pwm = 0;
      pendule.setCommand(pwm);
      // print on screen
      std::cout << "\r";
      std::cout << "  " << std::setw(12) << pendule.position();
      std::cout << "  " << std::setw(12) << pendule.angle();
      std::cout << "  " << std::setw(12) << pendule.linearVelocity();
      std::cout << "  " << std::setw(12) << pendule.angularVelocity();
      std::cout << "  " << std::setw(4) << pwm;
      std::cout << "   " << std::flush;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
