#pragma once
#include <vector>
#include <string>
#include <linux/joystick.h>

namespace pendule_pi {

/// Simple utility class to read information from a joystick.
class Joystick {
public:
  /// Connect to the joystick and initializes internal data.
  /** @param device file that should be used for the communication with the
    *   joystick.
    */
  Joystick(
    const std::string& device="/dev/input/js0"
  );
  /// Closes the connection with the joystick.
  ~Joystick();

  /// Read the current joystick state.
  bool update(bool process_all=true);

  /// Get the number of buttons on the Joystick.
  inline unsigned int nButtons() const { return buttons_.size(); };
  /// Get the number of axes on the Joystick.
  inline unsigned int nAxes() const { return axes_.size(); };
  /// Get the state of the given button.
  inline const int& button(int i) const { return buttons_.at(i); }
  /// Get the state of the given axis.
  inline const int& axis(int i) const { return axes_.at(i); }

private:
  int joy_file_descriptor_; ///< File descriptor to read the joystick state.
  std::vector<int> axes_; ///< Stores the state of each axis.
  std::vector<int> buttons_; ///< Stores the state of each button.
  std::string joy_name_; ///< Name of the joystick.
  js_event joy_event_; ///< Used to read joystick events.
};

}
