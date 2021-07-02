#include <pendule_pi/pigpio.hpp>
#include <pendule_pi/config.hpp>
#include <pendule_pi/debug.hpp>
#include <digital_filters/filters.hpp>
#include <chrono>
#include <thread>
#include "utils.hpp"


int main(int argc, char** argv) {
  namespace pp = pendule_pi;

  // Load all parameters from a configuration file.
  if(argc < 2) {
    std::cout << "Usage: sudo " << argv[0] << " config_file.yaml" << std::endl;
    std::cout << "- 'sudo' is needed for pigpio to work" << std::endl;
    std::cout << "- the configuration file is needed to setup the interface" << std::endl;
    return EXIT_FAILURE;
  }
  pp::Config config(argv[1]);

  try {
    // Let the token manage the pigpio library!
    pigpio::ActivationToken token;
    // Create the pendulum instance and perform the calibration.
    auto pendule = config.getPendule();
    std::cout << "Calibrating pendulum" << std::endl;
    pendule->calibrate(config.getCalibrationPWM());
    std::cout << "Calibration completed!" << std::endl;
    // Define soft limits for the pendulum.
    const double MAX_POSITION = pendule->softMinMaxPosition() - config.getSoftSafetyThreshold();
    // Create the timer used for enforcing a stable control rate.
    auto rate = config.getControlRate();
    const double PERIOD_SEC = rate->period()/1000000.0;
    // Setup filters.
    auto filter_position = config.getFilter();
    auto filter_angle = config.getFilter();
    auto filter_linvel = config.getFilter();
    auto filter_angvel = config.getFilter();
    filter_position->initInput(pendule->position());
    filter_position->initOutput(pendule->position());
    filter_angle->initInput(pendule->angle());
    filter_angle->initOutput(pendule->angle());
    filter_linvel->initInput(0.0);
    filter_linvel->initOutput(0.0);
    filter_angvel->initInput(0.0);
    filter_angvel->initOutput(0.0);
    // Control and process variables.
    int pwm = 0;
    double filtered_position;
    double filtered_angle;
    double filtered_linvel;
    double filtered_angvel;
    // Socket connections.
    auto tcp = config.getPenduleServer();
    const unsigned int MAX_MISSED_MESSAGES = 1 + static_cast<int>(1./PERIOD_SEC);

    // sleep a little bit before starting with the main loop
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    // Main loop!
    while(true) {
      // Sleep and update the state of the pendulum
      double hw_time = 1e-6 * rate->sleep();
      pendule->update(PERIOD_SEC);

      // perform state filtering
      filtered_position = filter_position->filter(pendule->position());
      filtered_angle = filter_angle->filter(pendule->angle());
      filtered_linvel = filter_linvel->filter(pendule->linearVelocity());
      filtered_angvel = filter_angvel->filter(pendule->angularVelocity());

      // send the current state
      tcp->sendState(
        hw_time,
        filtered_position,
        filtered_angle,
        filtered_linvel,
        filtered_angvel
      );

      // read the current command
      auto max_wait_time = std::min(0u, 1000*rate->residual() - 1);
      if(tcp->readCommand(max_wait_time)) {
        pwm = tcp->command();
      }
      else if(tcp->missedCommands() > MAX_MISSED_MESSAGES) {
        // we lost too many messages: override the command!
        pwm = 0;
      }

      // Enforce soft safety limits, then send the command.
      if(pendule->position() > MAX_POSITION && pwm > 0)
        pwm = 0;
      else if(pendule->position() < -MAX_POSITION && pwm < 0)
        pwm = 0;
      pendule->setCommand(pwm);
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return EXIT_SUCCESS;
}
