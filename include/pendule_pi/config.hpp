#pragma once
#include <yaml-cpp/yaml.h>

namespace pendule_pi {

/// Class that stores configuration parameters read from a YAML file.
class Config {
public:
  /// Returns a template configuration.
  static std::string getTemplate();
  /// Writes a template configuration into the given file.
  static void dumpTemplate(const std::string& path);
private:
};

} // pendule_pi
