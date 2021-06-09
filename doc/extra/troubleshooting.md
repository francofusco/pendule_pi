@page troubleshooting Troubleshooting

[TOC]

# The pendulum is moving

You might receive this message when starting the low-level interface. This might happen in two situations:

- As the message says, the rod and the weight are oscillating. Since we use incremental encoders, the pendulum has no knowledge about its configuration on startup. While it can locate itself by hitting the switches during the calibration process, to initialize the angular position we use the simple assumption that the pendulum initially points downward and does not move. Therefore, during calibration the angle of the pendulum will be monitored for a short period of time. If no motion is detected, then the angle is initialized to zero and the calibration procedure continues. Otherwise, an exception is thrown to tell you that *"The pendulum is moving"* @emoji :wink:
- The first time you launch the interface after powering it up. A false detection of encoder motions can happen when the GPIO module is configured for the first time since booting. Just relaunch the interface and hope for the best.


# Can't initialize pigpio library

See [pigpio's documentation](http://abyz.me.uk/rpi/pigpio/faq.html#Cant_initialise_pigpio_library). Most likely, you have the pigpio daemon (or another application using pigpio) running.


# DoxyGen configuration file

@warning @emoji :warning: Please, do *not* use the `doxywizard` (aka `doxygen-gui`) application to edit the Doxygen configuration file `doc/doxyfile.in`. In fact, it contains some CMake-configurable variables (such as `@GENERATE_LATEX@`) that would be overridden by `doxywizard` - it expects them to contain either `YES` or `NO` and freaks out if anything else is detected. The result is that it would break some of the functionalities of the CMake configuration.
