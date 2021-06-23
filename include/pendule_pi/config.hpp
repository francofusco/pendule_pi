#pragma once
#include <yaml-cpp/yaml.h>
#include <pendule_pi/switch.hpp>
#include <pendule_pi/encoder.hpp>
#include <pendule_pi/motor.hpp>
#include <pendule_pi/pendule.hpp>


namespace pendule_pi {

/// Class that stores configuration parameters read from a YAML file.
class Config {
public:
  /// Class to signal that a configuration file is missing a requested parameter.
  class MissingParameter : public std::runtime_error {
  public:
    /// Fill-in the exception message.
    MissingParameter(const std::string& param)
    : std::runtime_error("The configuration file does not contain the requiested parameter '" + param + "'")
    {}
  };

  /// Returns a template configuration.
  static std::string getTemplate();
  /// Writes a template configuration into the given file.
  static bool dumpTemplate(const std::string& path);

  /// Reads the configuration from the given YAML file.
  Config(const std::string& file_name);

  /// Returns a Switch instance given the configuration for the left switch.
  inline std::unique_ptr<Switch> getLeftSwitch() { return getSwitch("left_switch"); }

  /// Returns a Switch instance given the configuration for the right switch.
  inline std::unique_ptr<Switch> getRightSwitch() { return getSwitch("right_switch"); }

  /// Returns an Encoder instance given the configuration for the position encoder.
  inline std::unique_ptr<Encoder> getPositionEncoder() { return getEncoder("position_encoder"); }

  /// Returns an Encoder instance given the configuration for the angle encoder.
  inline std::unique_ptr<Encoder> getAngleEncoder() { return getEncoder("angle_encoder"); }

  /// Returns a Motor instance that is setup according to the configuration file.
  std::unique_ptr<Motor> getMotor();

  /// Returns a Pendule instance, setup according to the configuration file.
  std::unique_ptr<Pendule> getPendule();


private:
  /// Helper function to return a Switch instance.
  std::unique_ptr<Switch> getSwitch(const std::string& switch_name);
  /// Helper function to return a Encoder instance.
  std::unique_ptr<Encoder> getEncoder(const std::string& encoder_name);

  YAML::Node yaml_;
};

} // pendule_pi
