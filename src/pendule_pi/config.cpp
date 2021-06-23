#include <pendule_pi/config.hpp>
#include <fstream>

#define THROW_IF_MISSING_1(x) { if(!yaml_[x]) throw MissingParameter(x); }
#define THROW_IF_MISSING_2(x,y) { if(!yaml_[x][y]) throw MissingParameter(x + std::string("/") + y); }
#define THROW_IF_MISSING_3(x,y,z) { if(!yaml_[x][y][z]) throw MissingParameter(x + std::string("/") + y + std::string("/") + z); }

#define GET_MACRO(_1,_2,_3,NAME,...) NAME
#define THROW_IF_MISSING(...) GET_MACRO(__VA_ARGS__, THROW_IF_MISSING_3, THROW_IF_MISSING_2, THROW_IF_MISSING_1)(__VA_ARGS__)
// NOTE: add new "overloads" by adding _4 to the list in GET_MACRO and
// THROW_IF_MISSING_4 just before the __VA_ARGS__ in GET_MACRO

namespace pendule_pi {

bool Config::dumpTemplate(
  const std::string& path
)
{
  // Create the file writer.
  std::ofstream file(path);
  // Check if we managed to open it for writing. If not, exit!
  if(!file.is_open())
    return false;
  // Dump the YAML template into the file.
  file << getTemplate();
  file.close();
  return true;
}

std::string Config::getTemplate() {
  return std::string(
"# Configuration of the left switch (encoder side)\n"
"left_switch:\n"
"  # GPIO pin used to read the state of the switch.\n"
"  pin: 17\n"
"  # If true, then the pin should read 'high' when at rest.\n"
"  normally_up: true\n"
"  # If true, then the internal pull-up or pull-down resistor is activated.\n"
"  use_internal_pull_resistor: true\n"
"\n"
"# Configuration of the right switch (motor side).\n"
"right_switch:\n"
"  # GPIO pin used to read the state of the switch.\n"
"  pin: 18\n"
"  # If true, then the pin should read 'high' when at rest.\n"
"  normally_up: true\n"
"  # If true, then the internal pull-up or pull-down resistor is activated.\n"
"  use_internal_pull_resistor: true\n"
"\n"
"# Configuration of the position encoder (the one connected to\n"
"# the transmission belt).\n"
"position_encoder:\n"
"  # Pins corresponding to the two phases of the encoder.\n"
"  pin_a: 20\n"
"  pin_b: 21\n"
"\n"
"# Configuration of the angle encoder (the one that acts as\n"
"# the pivot for the rod).\n"
"angle_encoder:\n"
"  # Pins corresponding to the two phases of the encoder.\n"
"  pin_a: 19\n"
"  pin_b: 26\n"
"\n"
"# Configuration of the motor.\n"
"motor:\n"
"  # Pin used to send PWM signals to the motor controller.\n"
"  pwm_pin: 24\n"
"  # Pin used to invert the rotation direction of the motor.\n"
"  direction_pin: 16\n"
"\n"
"# Conversion coefficents for the encoders.\n"
"meters_per_step: 9.992676761711276e-06  # 0.846/84653\n"
"radians_per_step: 0.0015707963267948967  # 2*M_PI/4000\n"
"\n"
"# By default, the pendulum should read zero when in the downward position and PI\n"
"# when pointing upward. However, you might prefer a different convention, or\n"
"# simply ensure that the pendulum really stays at PI when it is manually\n"
"# balanced in the upward position. To achieve such result, the low level\n"
"# interface will subtract angle_offset from the \"raw\" reading.\n"
"angle_offset: 0.0\n"
"\n"
"# Safety distances to stop the pendulum, in meters.\n"
"safety_thresholds:\n"
"  # Minimum allowed distance from the switches. If violated, the interface\n"
"  # shuts-down. It should be positive. Negative values will be inverted.\n"
"  # Passing a zero is not illegal, but very unsafe.\n"
"  hard: 0.05\n"
"  # Minimum allowed distance from the hard safety threshold. If violated,\n"
"  # commands are zeroed. Giving a non-negative value in practice removes the\n"
"  # soft threshold (only the hard one will remain active).\n"
"  soft: 0.1\n"
"\n"
"# Offsets to be applied to pwm commands. Both should be integers.\n"
"pwm_offsets:\n"
"  # When a negative pwm is requested, actually apply pwm-low. If low is\n"
"  # negative, zero is used instead.\n"
"  low: 10\n"
"  # When a positive pwm is requested, actually apply pwm+high. If high is\n"
"  # negative, zero is used instead.\n"
"  high: 10\n"
"\n"
"# PWM command to be used when performing calibration.\n"
"calibration_pwm: 40"
"\n"
"# Control period in milliseconds (integer). Optional parameter, defaults to 20.\n"
"period_ms: 20\n"
"\n"
"# Cutoff frequency, in Hz, used in filtering. It should be less than half the\n"
"# sampling frequency. You can disable filtering by giving a negative value.\n"
"cutoff_frequency: 12.5\n"
"\n"
"# Configuration of the sockets used to exchange information.\n"
"sockets:\n"
"  # Host to bind the connections.\n"
"  host: \"*\"\n"
"  # Port to send the state to client applications.\n"
"  state_port: 10001\n"
"  # Port to receive commands from clients.\n"
"  command_port: 10002\n"
  );
}


Config::Config(
  const std::string& file_name
)
: yaml_(YAML::LoadFile(file_name))
{

}


std::unique_ptr<Switch> Config::getSwitch(
  const std::string& switch_name
)
{
  THROW_IF_MISSING(switch_name);
  THROW_IF_MISSING(switch_name, "pin");
  THROW_IF_MISSING(switch_name, "normally_up");
  THROW_IF_MISSING(switch_name, "use_internal_pull_resistor");
  return std::make_unique<Switch>(
    yaml_[switch_name]["pin"].as<int>(),
    yaml_[switch_name]["normally_up"].as<bool>(),
    yaml_[switch_name]["use_internal_pull_resistor"].as<bool>()
  );
}


std::unique_ptr<Encoder> Config::getEncoder(
  const std::string& encoder_name
)
{
  THROW_IF_MISSING(encoder_name);
  THROW_IF_MISSING(encoder_name, "pin_a");
  THROW_IF_MISSING(encoder_name, "pin_b");
  return std::make_unique<Encoder>(
    yaml_[encoder_name]["pin_a"].as<int>(),
    yaml_[encoder_name]["pin_b"].as<int>()
  );
}


std::unique_ptr<Motor> Config::getMotor()
{
  constexpr auto motor_name = "motor";
  THROW_IF_MISSING(motor_name);
  THROW_IF_MISSING(motor_name, "pwm_pin");
  THROW_IF_MISSING(motor_name, "direction_pin");
  return std::make_unique<Motor>(
    yaml_[motor_name]["pwm_pin"].as<int>(),
    yaml_[motor_name]["direction_pin"].as<int>()
  );
}


std::unique_ptr<Pendule> Config::getPendule()
{
  THROW_IF_MISSING("meters_per_step");
  THROW_IF_MISSING("radians_per_step");
  THROW_IF_MISSING("angle_offset");
  THROW_IF_MISSING("safety_thresholds");
  THROW_IF_MISSING("safety_thresholds", "hard");
  THROW_IF_MISSING("pwm_offsets");
  THROW_IF_MISSING("pwm_offsets", "low");
  THROW_IF_MISSING("pwm_offsets", "high");
  auto pendule = std::make_unique<Pendule>(
    yaml_["meters_per_step"].as<double>(),
    yaml_["radians_per_step"].as<double>(),
    yaml_["safety_thresholds"]["hard"].as<double>(),
    yaml_["angle_offset"].as<double>(),
    getMotor(),
    getLeftSwitch(),
    getRightSwitch(),
    getPositionEncoder(),
    getAngleEncoder()
  );
  pendule->setPwmOffsets(
    yaml_["pwm_offsets"]["low"].as<int>(),
    yaml_["pwm_offsets"]["high"].as<int>()
  );
  return pendule;
}

} // pendule_pi
