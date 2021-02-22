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
  const int button_plus_id = 0;
  const int button_minus_id = 1;

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
    // Assume the two buttons are at rest
    bool button_plus_past = false;
    bool button_minus_past = false;
    double MAX_POSITION = pendule.softMinMaxPosition() - 0.1;
    const int SLEEP_MS = 20;
    const double SLEEP_SEC = SLEEP_MS/1000.0;
    pigpio::Rate rate(SLEEP_MS*1000);
    std::cout << "Use the buttons #" << button_plus_id << " and #" << button_minus_id << " to increment / decrement the pwm signal" << std::endl;
    std::cout << "    POSITION        ANGLE       LIN.VEL.      ANG.VEL.     PWM" << std::endl;
                //  +000000.0000  +000000.0000  +000000.0000  +000000.0000  +000
    std::cout << std::setfill('0') << std::internal << std::showpos << std::fixed << std::setprecision(4);
    // desired pwm and actual command
    int pwm = 0;
    int cmd = 0;

    // main loop
    while(true) {
      // state estimation
      rate.sleep();
      pendule.update(SLEEP_SEC);
      // Update command from joy buttons
      joy.update();
      bool button_plus = joy.button(button_plus_id);
      bool button_minus = joy.button(button_minus_id);
      if(button_plus && !button_plus_past) {
        pwm++;
      }
      if(button_minus && !button_minus_past) {
        pwm--;
      }
      button_plus_past = button_plus;
      button_minus_past = button_minus;
      if(pendule.position() > MAX_POSITION && pwm > 0)
        cmd = 0;
      else if(pendule.position() < -MAX_POSITION && pwm < 0)
        cmd = 0;
      else
        cmd = pwm;
      pendule.setCommand(cmd);
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
