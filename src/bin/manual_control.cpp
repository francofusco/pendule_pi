#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/pendule.hpp"
#include "pendule_pi/joystick.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cmath>


int robustZero(int val, int zero) {
  if(-zero < val && val < zero)
    return 0;
  else
    return val;
}


int main(int argc, char** argv) {
  namespace pp = pendule_pi;
  if(argc != 2) {
    std::cout << "USAGE: " << argv[0] << " AXIS" << std::endl;
    std::cout << "  AXIS: number (0-based) of the axis to be used." << std::endl;
    std::exit(1);
  }
  const int axis_num = std::stoi(argv[1]);

  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the encoder
    pp::Joystick joy;
    const double ANGLE_OFFSET = 0.0;
    const double POSITION_RATIO_I3S = 0.846/21200;
    const double POSITION_RATIO_MIA = POSITION_RATIO_I3S*1000.0/600.0;
    pp::Pendule pendule(POSITION_RATIO_MIA, 2*M_PI/600, ANGLE_OFFSET);
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(0.05);
    std::cout << "Calibration completed!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    double MAX_POSITION = pendule.softMinMaxPosition() - 0.1;
    pendule.setPwmOffsets(20, 30);
    const int SLEEP_MS = 20;
    const double SLEEP_SEC = SLEEP_MS/1000.0;
    pigpio::Rate rate(SLEEP_MS*1000);
    std::cout << "    POSITION        ANGLE       LIN.VEL.      ANG.VEL.     PWM" << std::endl;
                //  +000000.0000  +000000.0000  +000000.0000  +000000.0000  +000
    std::cout << std::setfill('0') << std::internal << std::showpos << std::fixed << std::setprecision(4);
    while(true) {
      // state estimation
      rate.sleep();
      pendule.update(SLEEP_SEC);
      // Compute command
      joy.update();
      int ref = joy.axis(axis_num);
      ref = robustZero(ref, 2500);
      int cmd = static_cast<int>(ref*250./32768.);
      if(pendule.position() > MAX_POSITION && cmd > 0)
        cmd = 0;
      else if(pendule.position() < -MAX_POSITION && cmd < 0)
        cmd = 0;
      pendule.setCommand(cmd);
      // print on screen
      std::cout << "\r";
      std::cout << "  " << std::setw(12) << pendule.position();
      std::cout << "  " << std::setw(12) << pendule.angle();
      std::cout << "  " << std::setw(12) << pendule.linearVelocity();
      std::cout << "  " << std::setw(12) << pendule.angularVelocity();
      std::cout << "  " << std::setw(4) << cmd;
      std::cout << "   " << std::flush;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
