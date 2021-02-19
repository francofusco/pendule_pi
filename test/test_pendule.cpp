#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/pendule.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>
#include <cmath>


int main(int argc, char** argv) {
  namespace pp = pendule_pi;
  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the encoder
    pp::Pendule pendule(1.0, 2*M_PI/1000, 0.0);
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(2500.0);
    std::cout << "Calibration completed!" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    bool to_the_right = true;
    const int COMMAND = 75;
    double MAX_POSITION = pendule.softMinMaxPosition() - 1000.0;
    if(MAX_POSITION < 100.0) {
      throw std::runtime_error("MAX_POSITION is too small, this would cause serious troubles");
    }
    const int SLEEP_MS = 20;
    std::cout << "    POSITION        ANGLE       LIN.VEL.      ANG.VEL." << std::endl;
                //  +000000.0000  +000000.0000  +000000.0000  +000000.0000
    std::cout << std::setfill('0') << std::internal << std::showpos << std::fixed << std::setprecision(4);
    pendule.setCommand(COMMAND);
    while(true) {
      // state estimation
      std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_MS));
      pendule.update(SLEEP_MS/1000.0);
      // Compute command
      if(to_the_right && pendule.position() > MAX_POSITION) {
        pendule.setCommand(-COMMAND);
        to_the_right = false;
      }
      else if(!to_the_right && pendule.position() < -MAX_POSITION) {
        pendule.setCommand(COMMAND);
        to_the_right = true;
      }
      // print on screen
      std::cout << "\r";
      std::cout << "  " << std::setw(12) << pendule.position();
      std::cout << "  " << std::setw(12) << pendule.angle();
      std::cout << "  " << std::setw(12) << pendule.linearVelocity();
      std::cout << "  " << std::setw(12) << pendule.angularVelocity();
      std::cout << "   " << std::flush;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
