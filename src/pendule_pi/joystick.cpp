#include "pendule_pi/joystick.hpp"
#include "pendule_pi/debug.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>


namespace pendule_pi {

Joystick::Joystick(
	const std::string& device
)
: joy_file_descriptor_(-1)
, joy_name_("NO-NAME")
{
	PENDULE_PI_DBG("Creating Joystick instance connected to " << device);
	joy_file_descriptor_ = open( device.c_str() , O_RDONLY);
	if(joy_file_descriptor_ == -1)
		throw std::runtime_error("Failed to open " + device);
	// get name of the joystick
	const int NUM_CHARS = 80;
	char joy_name[NUM_CHARS];
	ioctl(joy_file_descriptor_, JSIOCGNAME(NUM_CHARS), &joy_name);
	joy_name_ = joy_name;
	PENDULE_PI_DBG("Joystick name: '" << joy_name_ << "'");
	// get number of axes and buttons
	int num_axes=0, num_buttons=0;
	ioctl(joy_file_descriptor_, JSIOCGAXES, &num_axes);
	ioctl(joy_file_descriptor_, JSIOCGBUTTONS, &num_buttons);
	PENDULE_PI_DBG("Detected " << num_buttons << " buttons and " << num_axes << " axes");
	axes_.resize(num_axes,0);
	buttons_.resize(num_buttons,0);
	// use non-blocking mode to read from the device
	fcntl(joy_file_descriptor_, F_SETFL, O_NONBLOCK);
	// Done!
	PENDULE_PI_DBG("Joystick instance created!");
}


Joystick::~Joystick() {
	PENDULE_PI_DBG("Destroyng Joystick instance '" << joy_name_ << "'");
	if(joy_file_descriptor_ != -1) {
		close( joy_file_descriptor_ );
		PENDULE_PI_DBG("File descriptor closed");
	}
	PENDULE_PI_DBG("Joystick object successfully closed");
}


bool Joystick::update(
	bool process_all
)
{
	bool something_changed = false;
	do {
		// read the current joystick state
		auto retval = read(joy_file_descriptor_, &joy_event_, sizeof(js_event));
		// check the result of the read operation
		if(retval < 0) {
			// Negative values mean error! However, it seems that EWOULDBLOCK (-11)
			// is returned when performing non-blocking reads
			if(errno == EWOULDBLOCK) {
				break;
			}
			else {
				throw std::runtime_error(
					"Failed to read joystick state (read returned " + std::to_string(retval)
					+	"). Error: " + std::string(std::strerror(errno)) + " (" +
					std::to_string(errno) + ")"
				);
			}
		}
		else if(retval == 0) {
			// Nothing read, we can exit.
			break;
		}
		// We got an event, process it!
		if(joy_event_.type == JS_EVENT_AXIS) {
			axes_.at(joy_event_.number) = joy_event_.value;
			something_changed = true;
		}
		else if(joy_event_.type == JS_EVENT_BUTTON) {
			buttons_.at(joy_event_.number) = joy_event_.value;
			something_changed = true;
		}
	}
	while(process_all);
	return something_changed;
}

}
