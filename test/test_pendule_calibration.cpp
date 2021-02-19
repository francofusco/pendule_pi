#include "pendule_pi/pendule.hpp"
#include "pendule_pi/pigpio.hpp"
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
    // create the components as pointer
    auto motor = std::make_unique<pp::Motor>(24, 16);
    auto right_switch = std::make_unique<pp::Switch>(18, pp::Switch::NORMALLY_UP, pp::Switch::WITH_PULL_RESISTOR);
    auto left_switch = std::make_unique<pp::Switch>(17, pp::Switch::NORMALLY_UP, pp::Switch::WITH_PULL_RESISTOR);
    auto position_encoder = std::make_unique<pp::Encoder>(20, 21);
    auto angle_encoder = std::make_unique<pp::Encoder>(19, 26);
    // Create the encoder with the list of components
    pp::Pendule pendule(
      1.0, 2*M_PI/1000, 0.0,
      std::move(motor),
      std::move(left_switch),
      std::move(right_switch),
      std::move(position_encoder),
      std::move(angle_encoder)
    );
    // calibrate the pendulum, then print some data
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(1000.0);
    std::cout << "Calibration completed!" << std::endl;
    std::cout << "Min steps: " << pendule.minPositionSteps() << std::endl;
    std::cout << "Mid steps: " << pendule.midPositionSteps() << std::endl;
    std::cout << "Max steps: " << pendule.maxPositionSteps() << std::endl;
    std::cout << "Step range: " << (pendule.maxPositionSteps()-pendule.minPositionSteps()) << std::endl;
    std::cout << "Soft position limits: " << pendule.softMinMaxPosition() << std::endl;
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
