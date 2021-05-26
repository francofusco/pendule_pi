#include <pendule_pi/pigpio.hpp>
#include <pendule_pi/pendule.hpp>
#include <digital_filters/filters.hpp>
#include <zmqpp/zmqpp.hpp>
#include <chrono>
#include <thread>
#include "utils.hpp"


int main(int argc, char** argv) {
  namespace pp = pendule_pi;

  try {
    // Let the token manage the pigpio library!
    pigpio::ActivationToken token;
    // Create the pendulum instance and perform the calibration.
    pp::Pendule pendule(0.846/21200, 2*M_PI/1000, 0.0);
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(0.05);
    pendule.setPwmOffsets(13, 17);
    std::cout << "Calibration completed!" << std::endl;
    // Define soft limits for the pendulum.
    const double MAX_POSITION = pendule.softMinMaxPosition() - 0.1;
    // Create the timer used for enforcing a stable control rate.
    const int SLEEP_MS = 20;
    const double SLEEP_SEC = SLEEP_MS/1000.0;
    pigpio::Rate rate(SLEEP_MS*1000);
    // Variables used to perform control and filtering
    int pwm = 0;
    // Filters
    double Fs = 1.0/SLEEP_SEC; // sampling frequency
    double Fc = Fs/4; // cutoff frequency
    auto filter_position = digital_filters::butterworth<double,double>(4, Fc, Fs);
    auto filter_angle = digital_filters::butterworth<double,double>(4, Fc, Fs);
    auto filter_linvel = digital_filters::butterworth<double,double>(4, Fc, Fs);
    auto filter_angvel = digital_filters::butterworth<double,double>(4, Fc, Fs);
    filter_position.initInput(pendule.position());
    filter_position.initOutput(pendule.position());
    filter_angle.initInput(pendule.angle());
    filter_angle.initOutput(pendule.angle());
    filter_linvel.initInput(0.0);
    filter_linvel.initOutput(0.0);
    filter_angvel.initInput(0.0);
    filter_angvel.initOutput(0.0);
    auto filter_target_position = digital_filters::butterworth<double,double>(2, Fs/20, Fs);
    // process variables
    double filtered_position;
    double filtered_angle;
    double filtered_linvel;
    double filtered_angvel;
    // Create the socket connections
    zmqpp::context context;
    zmqpp::socket state_pub(context, zmqpp::socket_type::publish);
    state_pub.bind("tcp://*:10001");
    zmqpp::socket command_sub(context, zmqpp::socket_type::subscribe);
    command_sub.set(zmqpp::socket_option::conflate, 1);
    command_sub.bind("tcp://*:10002");
    command_sub.subscribe("");
    // sleep a little bit before starting with the main loop
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // Main loop!
    while(true) {
      // Sleep and update the state of the pendulum
      double hw_time = 1e-6 * rate.sleep();
      pendule.update(SLEEP_SEC);
      // perform state filtering
      filtered_position = filter_position.filter(pendule.position());
      filtered_angle = filter_angle.filter(pendule.angle());
      filtered_linvel = filter_linvel.filter(pendule.linearVelocity());
      filtered_angvel = filter_angvel.filter(pendule.angularVelocity());
      // send the current state
      zmqpp::message msg;
      msg << std::to_string(hw_time) + " "
           + std::to_string(filtered_position) + " "
           + std::to_string(filtered_angle) + " "
           + std::to_string(filtered_linvel) + " "
           + std::to_string(filtered_angvel);
      state_pub.send(msg, true);
      // read the current command
      if(command_sub.receive(msg, true)) {
        std::string msg_str;
        msg >> msg_str;
        pwm = std::stoi(msg_str);
      }
      // Enforce soft safety limits, then send the command.
      if(pendule.position() > MAX_POSITION && pwm > 0)
        pwm = 0;
      else if(pendule.position() < -MAX_POSITION && pwm < 0)
        pwm = 0;
      pendule.setCommand(pwm);
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
