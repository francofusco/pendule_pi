/** @file pigpio.hpp
  * @brief Header containing utilities to extend pigpio C++ API.
  */
#pragma once

#include <stdexcept>
#include <map>
#include <pigpio.h>


/// Wrapper macro that throws a PiGPIOError if the given pigpio function fails.
/** This macro wraps over pigpio functions so that an exception is thrown upon
  * return of a negative error code. As an example, you can write:
  * ```
  * int state;
  * PiGPIO_RUN(state, gpioRead, 10);
  * ```
  * Which is almost equivalent to
  * ```
  * int state;
  * state = gpioRead(10);
  * ```
  * The difference is that in the first case if `gpioRead` returns a negative
  * value then a PiGPIOError is thrown.
  * @param retval an integer variable used to store the result of the function
  *   call.
  * @param func the pigpio function to be run.
  * @param ... arguments to be passed to func.
  */
#define PiGPIO_RUN(retval, func, ...) { retval = func(__VA_ARGS__); if(retval < 0) throw pigpio::Exception(#func, retval); }

/// Wrapper macro that throws a PiGPIOError if the given pigpio function fails.
/** Similar to PiGPIO_RUN(retval, func, ...), but it act as if the pigpio
  * function to be called was void. With this macro, you can write codes like:
  * ```
  * PiGPIO_RUN_VOID(gpioSetMode, 10, PI_OUTPUT); // amost equal to gpioSetMode(10, PI_INPUT);
  * PiGPIO_RUN_VOID(gpioWrite, 10, PI_HIGH); // amost equal to gpioWrite(10, PI_HIGH);
  * ```
  * @param func the pigpio function to be run.
  * @param ... arguments to be passed to func.
  */
#define PiGPIO_RUN_VOID(func, ...) { int retval; PiGPIO_RUN(retval, func, __VA_ARGS__); }


/// Namespace grouping the "extra" pigpio functionalities.
namespace pigpio {

/// Simple exception wrapper over pigpio's error codes.
class Exception : public std::runtime_error {
public:
  /// Initialize the exception given the function and the error code.
  Exception(
    const std::string& caller,
    int return_code
  );

  /// Returns a human-readable description of the given error code.
  /** @param error_code an error code that can be produced by one of pigpio's
    *   functions.
    * @return The human-readable correspondant of error_code, or a dummy text
    *   if the code is unknown.
    */
  static std::string error2msg(int error_code);

private:
  /// Auxiliary static method that creates exception messages.
  /** @param caller the name of the pigpio function that caused the error.
    * @param return_code the (negative) value returned by the pigpio function.
    * @return a human-readable string in which the error code is replaced by
    *   its name as defined in
    *   <a href="https://github.com/joan2937/pigpio/blob/db1fd9cf6c3431314b04edc486d900151a0dfd78/pigpio.h#L6389">pigpio's header.</a>
    */
  static std::string makeMessage(
    const std::string& caller,
    int return_code
  );

  /// Map from a pigpio's error code to its human-readable name.
  static const std::map<int,std::string> ERROR_CODES;
};

/// Utility class that activates and deactivates the pigpio library.
/** This class helps to initialize and terminate the pigpio library.
  * Upon construction, this class calls gpioInitialise(), while upon destruction
  * it calls gpioTerminate(). This allows to write codes such as the following,
  * which should ensure that the library is "closed" even if an exception
  * happens:
  * ```
  * int main() {
  *   // code that does not need pigpio
  *   try {
  *     pigpio::ActivationToken token;
  *     // execute your pigpio-based instructions here!
  *   }
  *   catch(const pigpio::ActivationToken::Terminate&) {  }
  *   catch(...) { throw; }
  *   return 0;
  * }
  * ```
  */
class ActivationToken {
public:
  /// Forwards a call to gpioInitialise().
  /** The constructor also registers the static method terminate() as handler
    * for SIGINT (the one sent when CTRL+C is pressed on the keyboard).
    */
  ActivationToken();
  /// Forwards a call to gpioTerminate().
  ~ActivationToken();

  /// Auxiliary exception class to be thrown when a SIGINT is received.
  class PleaseStop : public std::runtime_error {
    public: PleaseStop() : std::runtime_error("SIGINT received, throwing this exception to stop excution") {}
  };
private:
  /// Puts all GPIO pins in high impedance mode.
  /** This method allows to "disconnect" all GPIO pins, by putting them into
    * high impedance mode (inputs with no pull-up/pull-down resistors).
    * Additionally, this method calls `gpioTerminate()` so that further
    * GPIO operations will not happen.
    */
  static void resetPins();

  /// Static method that can be registered as handler for SIGINT (CTRL+C).
  /** This method simply throws a PleaseStop exception to halt execution.
    * @param sig signal to be handled. Ignored in this method, but required
    *   by the syntax of std::signal.
    */
  static void pleaseStop(int sig);

  /// Static method that can be registered as handler for SIGABRT.
  /** This method calls .
    * @param sig signal to be handled. Ignored in this method, but required
    *   by the syntax of std::signal.
    */
  static void abort(int sig);
};

} // end of namespace pigpio
