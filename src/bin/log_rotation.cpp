#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/encoder.hpp"
#include "pendule_pi/joystick.hpp"
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>


bool moving(
  const pendule_pi::Encoder& encoder,
  unsigned int wait_seconds
)
{
  unsigned int wait_us = wait_seconds * 1000000;
  unsigned int t0 = gpioTick();
  while(gpioTick() - t0 < wait_us) {
    if(encoder.steps() != 0)
      return true;
  }
  return false;
}


int main(int argc, char** argv) {
  namespace pp = pendule_pi;

  // Logging settings
  const unsigned int PERIOD_US = 5000;
  const unsigned int DATA_PACKET_SIZE = 1000;

  // Open the log file and write the header
  const std::string data_file_name = argc>1 ? argv[1] : "logged_angles.csv";
  std::ofstream data_file(data_file_name, std::ios::out);
  if(!data_file.is_open()) {
    std::cout << "ERROR! Could not open file '" << data_file_name << "'" << std::endl;
    return 1;
  }
  data_file << "packet,time_us, angle" << std::endl;
  data_file.close();

  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Encoder for the angle
    pp::Encoder encoder(19, 26);
    // Joystick to guide the acquisition
    pp::Joystick joy("/dev/input/js0");

    // Check that the pendulum stays at zero
    if(moving(encoder,2)) {
      throw std::runtime_error("The encoder is moving!");
    }

    // Rate used for logging
    pigpio::Rate rate(PERIOD_US);

    // Main loop
    unsigned int packet_id = 0;
    while(true) {
      // Prepare the data
      std::vector<unsigned int> times;
      std::vector<int> angles;
      times.reserve(DATA_PACKET_SIZE);
      angles.reserve(DATA_PACKET_SIZE);

      // Wait for joystick input before logging
      std::cout << "Press the button #0 on the joystick to start logging! (CTRL+C to exit)" << std::endl;
      do {
        joy.update();
      }
      while(joy.button(0) == 0);
      std::cout << "Logging, please wait... " << std::flush;

      // Record data
      unsigned int tnow = 0;
      while(times.size() < DATA_PACKET_SIZE) {
        // Wait for the clock to advance
        tnow = rate.sleep();
        // Do logging
        angles.push_back(encoder.steps());
        times.push_back(tnow);
      }
      std::cout << "Done!" << std::endl;

      // Store data inside the file
      std::cout << "Writing data, DO NOT EXIT! " << std::flush;
      data_file.open(data_file_name, std::ofstream::out | std::ofstream::app);
      if(!data_file.is_open()) {
        throw std::runtime_error("Could not open file '" + data_file_name + "'");
      }
      for(unsigned int i=0; i<DATA_PACKET_SIZE; i++) {
        data_file << packet_id << ", " << times[i] << ", " << angles[i] << std::endl;
      }
      data_file.close();
      std::cout << "Thanks for your patience :)" << std::endl;
      packet_id++;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
