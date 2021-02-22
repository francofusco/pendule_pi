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


int main(int argc, char** argv) {
  namespace pp = pendule_pi;
  if(argc != 2) {
    std::cout << "USAGE: " << argv[0] << " AXIS" << std::endl;
    std::cout << "  AXIS: number (0-based) of the axis to be used." << std::endl;
    std::exit(1);
  }
  const int axis_num = std::stoi(argv[1]);

  // Open the log file and write the header
  const std::string data_file_name = "logged_joystick.csv";
  std::ofstream data_file(data_file_name, std::ios::out);
  if(!data_file.is_open()) {
    std::cout << "ERROR! Could not open file '" << data_file_name << "'" << std::endl;
    return 1;
  }
  data_file << "time_us, pwm, position, angle" << std::endl;
  data_file.close();

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
    const int SLEEP_US = 10000;
    const double SLEEP_SEC = SLEEP_US / 1000000.0;
    pigpio::Rate rate(SLEEP_US);

    while(true) {
      // Wait for joystick input before logging
      std::cout << "Press the button #0 on the joystick to start logging! (CTRL+C to exit)" << std::endl;
      do {
        joy.update();
      }
      while(joy.button(0) == 0);

      // Prepare the data
      unsigned int t;
      int pwm = 0;
      const unsigned int MAX_DATA = 1000;
      std::vector<unsigned int> times;
      std::vector<int> commands;
      std::vector<double> positions;
      std::vector<double> angles;
      times.reserve(MAX_DATA);
      commands.reserve(MAX_DATA);
      angles.reserve(MAX_DATA);
      positions.reserve(MAX_DATA);

      // state estimation
      while(times.size() < MAX_DATA) {
        t = rate.sleep();
        pendule.update(SLEEP_SEC);
        times.push_back(t);
        commands.push_back(pwm);
        positions.push_back(pendule.position());
        angles.push_back(pendule.angle());
        // command
        joy.update();
        int ref = joy.axis(axis_num);
        ref = robustZero(ref, 2500);
        pwm = (int)(ref*250./32768.);
        if(pendule.position() > MAX_POSITION && pwm > 0)
          pwm = 0;
        else if(pendule.position() < -MAX_POSITION && pwm < 0)
          pwm = 0;
        pendule.setCommand(pwm);
      }
      pendule.setCommand(0);

      // Store data inside the file
      std::cout << "Writing data, DO NOT EXIT! " << std::flush;
      data_file.open(data_file_name, std::ofstream::out | std::ofstream::app);
      if(!data_file.is_open()) {
        throw std::runtime_error("Could not open file '" + data_file_name + "'");
      }
      for(unsigned int i=0; i<times.size(); i++) {
        data_file << times[i] << ", " << commands[i] << ", " << positions[i] << ", " << angles[i] << std::endl;
      }
      data_file.close();
      std::cout << "Thanks for your patience :)" << std::endl;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
