#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/pendule.hpp"
#include "pendule_pi/joystick.hpp"
#include <iostream>
#include <fstream>
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


std::vector<int> range(int start, int stop, int step) {
  std::vector<int> vec;
  for(int i=start; i<=stop; i+=step)
    vec.push_back(i);
  return vec;
}


int main(int argc, char** argv) {
  namespace pp = pendule_pi;

  // Logging settings
  const unsigned int PERIOD_US = 5000;
  const double PERIOD_SEC = PERIOD_US / 1000000.0;

  // Open the log file and write the header
  const std::string data_file_name = argc>1 ? argv[1] : "logged_motion.csv";
  std::ofstream data_file(data_file_name, std::ios::out);
  if(!data_file.is_open()) {
    std::cout << "ERROR! Could not open file '" << data_file_name << "'" << std::endl;
    return 1;
  }
  data_file << "pwm, time_us, position, angle" << std::endl;
  data_file.close();

  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the encoder
    pp::Pendule pendule(0.846/21200, 2*M_PI/1000, 0.0);
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(0.05);
    std::cout << "Calibration completed!" << std::endl;
    double MAX_POSITION = pendule.softMinMaxPosition() - 0.1;
    pendule.setPwmOffsets(20, 30);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    pigpio::Rate rate(PERIOD_US);

    // Go to the left
    pendule.setCommand(-50);
    do {
      rate.sleep();
      pendule.update(PERIOD_SEC);
    }
    while(pendule.position() > -MAX_POSITION);
    pendule.setCommand(0);

    const int ESTIM_DATA = static_cast<int>(20 * 1000000 / PERIOD_US);
    std::vector<int> commands = range(50, 250, 10);
    unsigned int t;

    for(const auto& pwm : commands) {
      // Prepare the data
      std::vector<unsigned int> times;
      std::vector<double> positions;
      std::vector<double> angles;
      times.reserve(ESTIM_DATA);
      angles.reserve(ESTIM_DATA);
      positions.reserve(ESTIM_DATA);

      std::cout << "PWM: " << pwm << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      // move to the right
      pendule.setCommand(pwm);
      do {
        t = rate.sleep();
        pendule.update(PERIOD_SEC);
        times.push_back(t);
        positions.push_back(pendule.position());
        angles.push_back(pendule.angle());
      }
      while(pendule.position() < MAX_POSITION);
      pendule.setCommand(0);

      // Store data inside the file
      std::cout << "Writing data, DO NOT EXIT! " << std::flush;
      data_file.open(data_file_name, std::ofstream::out | std::ofstream::app);
      if(!data_file.is_open()) {
        throw std::runtime_error("Could not open file '" + data_file_name + "'");
      }
      for(unsigned int i=0; i<times.size(); i++) {
        data_file << pwm << ", " << times[i] << ", " << positions[i] << ", " << angles[i] << std::endl;
      }
      data_file.close();
      std::cout << "Written " << times.size() << " samples" << std::endl;
      std::cout << "Thanks for your patience :)" << std::endl;

      // reset buffers
      times.clear();
      angles.clear();
      positions.clear();
      times.reserve(ESTIM_DATA);
      angles.reserve(ESTIM_DATA);
      positions.reserve(ESTIM_DATA);

      std::cout << "PWM: " << -pwm << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));

      // move to the left
      pendule.setCommand(-pwm);
      do {
        t = rate.sleep();
        pendule.update(PERIOD_SEC);
        times.push_back(t);
        positions.push_back(pendule.position());
        angles.push_back(pendule.angle());
      }
      while(pendule.position() > -MAX_POSITION);
      pendule.setCommand(0);

      // Store data inside the file
      std::cout << "Writing data, DO NOT EXIT! " << std::flush;
      data_file.open(data_file_name, std::ofstream::out | std::ofstream::app);
      if(!data_file.is_open()) {
        throw std::runtime_error("Could not open file '" + data_file_name + "'");
      }
      for(unsigned int i=0; i<times.size(); i++) {
        data_file << -pwm << ", " << times[i] << ", " << positions[i] << ", " << angles[i] << std::endl;
      }
      data_file.close();
      std::cout << "Written " << times.size() << " samples" << std::endl;
      std::cout << "Thanks for your patience :)" << std::endl;

    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
