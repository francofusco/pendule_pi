#include <yaml-cpp/yaml.h>
#include <iostream>

int main(int argc, char** argv) {
  const std::string CONFIG_FILE_DEFAULT("~/.pendule_pi_config.yaml");
  std::string config_file = argc < 2 ? CONFIG_FILE_DEFAULT : std::string(argv[1]);

  YAML::Node config = YAML::LoadFile(config_file);
  std::cout << "meters_per_step: " << config["meters_per_step"].as<double>() << std::endl;
  std::cout << "radians_per_step: " << config["radians_per_step"].as<double>() << std::endl;
  std::cout << "safety_thresholds_hard: " << config["safety_thresholds"]["hard"].as<double>() << std::endl;
  std::cout << "safety_thresholds_soft: " << config["safety_thresholds"]["soft"].as<double>() << std::endl;
  std::cout << "pwm_offsets: low " << config["pwm_offsets"]["low"].as<int>() << std::endl;
  std::cout << "pwm_offsets: high " << config["pwm_offsets"]["high"].as<int>() << std::endl;
  std::cout << "period_ms: " << config["period_ms"].as<int>() << std::endl;
  std::cout << "cutoff_frequency: " << config["cutoff_frequency"].as<double>() << std::endl;

  return EXIT_SUCCESS;
}
