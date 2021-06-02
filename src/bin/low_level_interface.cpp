#include <pendule_pi/pigpio.hpp>
#include <pendule_pi/pendule.hpp>
#include <pendule_pi/debug.hpp>
#include <digital_filters/filters.hpp>
#include <yaml-cpp/yaml.h>
#include <zmqpp/zmqpp.hpp>
#include <chrono>
#include <thread>
#include "utils.hpp"


int main(int argc, char** argv) {
  namespace pp = pendule_pi;

  // Load all parameters from a configuration file.
  const std::string config_file = argc > 1 ? argv[1] : "./pendule_pi_config.yaml";
  YAML::Node config = YAML::LoadFile(config_file);
  // Get pin "layouts"
  pp::Pendule::Pins pins;
  if(config["pins"]) {
    if(config["pins"]["motor"]) {
      if(config["pins"]["motor"]["pwm"])
        pins.motor_pwm = config["pins"]["motor"]["pwm"].as<int>();
      if(config["pins"]["motor"]["direction"])
        pins.motor_dir = config["pins"]["motor"]["direction"].as<int>();
    }
    if(config["pins"]["left_switch"])
      pins.left_switch = config["pins"]["left_switch"].as<int>();
    if(config["pins"]["right_switch"])
      pins.right_switch = config["pins"]["right_switch"].as<int>();
    if(config["pins"]["position_encoder"]) {
      if(config["pins"]["position_encoder"]["a"])
        pins.position_encoder_a = config["pins"]["position_encoder"]["a"].as<int>();
      if(config["pins"]["position_encoder"]["b"])
        pins.position_encoder_b = config["pins"]["position_encoder"]["b"].as<int>();
    }
    if(config["pins"]["angle_encoder"]) {
      if(config["pins"]["angle_encoder"]["a"])
        pins.angle_encoder_a = config["pins"]["angle_encoder"]["a"].as<int>();
      if(config["pins"]["angle_encoder"]["b"])
        pins.angle_encoder_b = config["pins"]["angle_encoder"]["b"].as<int>();
    }
  }
  // Sockets configuration
  std::string HOST("*");
  std::string STATE_PORT("10001");
  std::string COMMAND_PORT("10002");
  double MAX_IDLE_TIME = 1.0;
  if(config["sockets"]) {
    if(config["sockets"]["host"])
      HOST = config["sockets"]["host"].as<std::string>();
    if(config["sockets"]["state_port"])
      STATE_PORT = config["sockets"]["state_port"].as<std::string>();
    if(config["sockets"]["command_port"])
      COMMAND_PORT = config["sockets"]["command_port"].as<std::string>();
    if(config["sockets"]["max_idle_time"])
      MAX_IDLE_TIME = config["sockets"]["max_idle_time"].as<double>();
  }
  // Get other configuration parameters.
  const auto METERS_PER_STEP = config["meters_per_step"].as<double>();
  const auto RADIANS_PER_STEP = config["radians_per_step"].as<double>();
  const auto ANGLE_OFFSET = config["angle_offset"] ? config["angle_offset"].as<double>() : 0.0;
  const auto SAFETY_THRESHOLD_HARD = config["safety_thresholds"]["hard"].as<double>();
  const auto SAFETY_THRESHOLD_SOFT = config["safety_thresholds"]["soft"].as<double>();
  const auto PWM_OFFSET_LOW = config["pwm_offsets"]["low"].as<int>();
  const auto PWM_OFFSET_HIGH = config["pwm_offsets"]["high"].as<int>();
  const auto PERIOD_MS = config["period_ms"] ? config["period_ms"].as<int>() : 20;
  const auto CUTOFF_FREQUENCY = config["cutoff_frequency"].as<double>();
  // In debug mode
  PENDULE_PI_DBG("LOW-LEVEL INTERFACE CONFIGURATION:");
  PENDULE_PI_DBG("----------------------------------");
  PENDULE_PI_DBG("PINS");
  PENDULE_PI_DBG("motor:");
  PENDULE_PI_DBG("  pwm: " << pins.motor_pwm);
  PENDULE_PI_DBG("  dir: " << pins.motor_dir);
  PENDULE_PI_DBG("left switch: " << pins.left_switch);
  PENDULE_PI_DBG("right switch: " << pins.right_switch);
  PENDULE_PI_DBG("position encoder:");
  PENDULE_PI_DBG("  a: " << pins.position_encoder_a);
  PENDULE_PI_DBG("  b: " << pins.position_encoder_b);
  PENDULE_PI_DBG("angle encoder:");
  PENDULE_PI_DBG("  a: " << pins.angle_encoder_a);
  PENDULE_PI_DBG("  b: " << pins.angle_encoder_b);
  PENDULE_PI_DBG("----------------------------------");
  PENDULE_PI_DBG("PENDULUM PARAMETERS");
  PENDULE_PI_DBG("meters per step: " << METERS_PER_STEP);
  PENDULE_PI_DBG("radians per step: " << RADIANS_PER_STEP);
  PENDULE_PI_DBG("angle offset: " << ANGLE_OFFSET);
  PENDULE_PI_DBG("safety thresholds:");
  PENDULE_PI_DBG("  hard: " << SAFETY_THRESHOLD_HARD);
  PENDULE_PI_DBG("  soft: " << SAFETY_THRESHOLD_SOFT);
  PENDULE_PI_DBG("PWM offsets:");
  PENDULE_PI_DBG("  low: " << PWM_OFFSET_LOW);
  PENDULE_PI_DBG("  high: " << PWM_OFFSET_HIGH);
  PENDULE_PI_DBG("period [ms]: " << PERIOD_MS);
  PENDULE_PI_DBG("cutoff frequency [Hz]: " << CUTOFF_FREQUENCY);
  PENDULE_PI_DBG("----------------------------------");
  PENDULE_PI_DBG("SOCKETS");
  PENDULE_PI_DBG("host: " << HOST);
  PENDULE_PI_DBG("state port: " << STATE_PORT);
  PENDULE_PI_DBG("command port: " << COMMAND_PORT);
  PENDULE_PI_DBG("----------------------------------");

  try {
    // Let the token manage the pigpio library!
    pigpio::ActivationToken token;
    // Create the pendulum instance and perform the calibration.
    pp::Pendule pendule(METERS_PER_STEP, RADIANS_PER_STEP, ANGLE_OFFSET, pins);
    std::cout << "Calibrating pendulum" << std::endl;
    pendule.calibrate(SAFETY_THRESHOLD_HARD);
    pendule.setPwmOffsets(PWM_OFFSET_LOW, PWM_OFFSET_HIGH);
    std::cout << "Calibration completed!" << std::endl;
    // Define soft limits for the pendulum.
    const double MAX_POSITION = pendule.softMinMaxPosition() - SAFETY_THRESHOLD_SOFT;
    // Create the timer used for enforcing a stable control rate.
    const double PERIOD_SEC = PERIOD_MS/1000.0;
    pigpio::Rate rate(PERIOD_MS*1000);
    // Variables used to perform control and filtering
    int pwm = 0;
    // Filters
    const double Fs = 1.0/PERIOD_SEC; // sampling frequency
    auto filter_position = digital_filters::butterworth<double,double>(4, CUTOFF_FREQUENCY, Fs);
    auto filter_angle = digital_filters::butterworth<double,double>(4, CUTOFF_FREQUENCY, Fs);
    auto filter_linvel = digital_filters::butterworth<double,double>(4, CUTOFF_FREQUENCY, Fs);
    auto filter_angvel = digital_filters::butterworth<double,double>(4, CUTOFF_FREQUENCY, Fs);
    filter_position.initInput(pendule.position());
    filter_position.initOutput(pendule.position());
    filter_angle.initInput(pendule.angle());
    filter_angle.initOutput(pendule.angle());
    filter_linvel.initInput(0.0);
    filter_linvel.initOutput(0.0);
    filter_angvel.initInput(0.0);
    filter_angvel.initOutput(0.0);
    // process variables
    double filtered_position;
    double filtered_angle;
    double filtered_linvel;
    double filtered_angvel;
    // Create the socket connections
    zmqpp::context context;
    zmqpp::socket state_pub(context, zmqpp::socket_type::publish);
    state_pub.bind("tcp://" + HOST + ":" + STATE_PORT);
    zmqpp::socket command_sub(context, zmqpp::socket_type::subscribe);
    command_sub.set(zmqpp::socket_option::conflate, 1);
    command_sub.bind("tcp://" + HOST + ":" + COMMAND_PORT);
    command_sub.subscribe("");
    const unsigned int MAX_MISSED_MESSAGES = 1 + static_cast<int>(MAX_IDLE_TIME/PERIOD_SEC);
    unsigned int missed_messages = 0;
    // sleep a little bit before starting with the main loop
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // Main loop!
    while(true) {
      // Sleep and update the state of the pendulum
      double hw_time = 1e-6 * rate.sleep();
      pendule.update(PERIOD_SEC);
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
        missed_messages = 0;
      }
      else if(missed_messages < MAX_MISSED_MESSAGES) {
        // no command was available, but we did not "loose" too many messages
        missed_messages++;
      }
      else {
        // we lost too many messages: override the command!
        pwm = 0;
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

  return EXIT_SUCCESS;
}
