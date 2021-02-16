/** @file switch.hpp
  * @brief Header file for the Switch class.
  */
#pragma once

#include <functional>
#include <stdexcept>

namespace pendule_pi {

/// Class that handles a simple mechanical switch.
class Switch {
public:
  /// Constructor, initialized the associated GPIO pin.
  /** @param pin number of the pin used by the switch.
    * @param normally_up if true, when the switch is at rest the pin should be
    *   in a logical HIGH. If false, when the switch is at rest the pin should
    *   be in a logical HIGH.
    * @param with_pull_up_down if true, the GPIO pin will be configured so that
    *   it uses the pull-up or a pull-down resistor. If false, no additional
    *   resistor is used.
    * @warning If you pass `with_pull_up_down=true`, then the type of resistor
    *   (pull-up vs pull-down) is decided via the `normally_up` parameter. In
    *   particular:
    *   - if `normally_up=true` then the pull-up resistor is enabled;
    *   - if `normally_up=false` then the pull-down resistor is enabled.
    *   Make sure that your circuitry coupled with these choices does not damage
    *   the board!
    */
  Switch(
    int pin,
    bool normally_up,
    bool with_pull_up_down=true
  );

  /// Destructor, disconnects the GPIO callback.
  virtual ~Switch();

  /// Enables GPIO interrupts on the connected pin.
  /** When interrupts are enabled, the current state of the switch is updated
    * autonomously.
    * @param user_callback optional callback to be executed whenever the switch
    *   becomes active.
    */
  void enableInterrupts(
    std::function<void(void)> user_callback = nullptr
  );

  /// Disables GPIO interrupts on the connected pin.
  /** This method also resets to `nullptr` eventual user callbacks previously
    * passed to enableInterrupts().
    */
  void disableInterrupts();

  /// Get the current state of the switch.
  /** This method performs a "direct" GPIO read, *i.e.*, it does not use the
    * "cached" state (obtained using interrupts).
    * @return true if the switch is at rest and false otherwise.
    */
  bool atRest() const;

  /// Get the current state of the switch.
  /** This method returns the "cached" state of the pin, which should have been
    * updated using GPIO interrupts.
    * @return true if the switch is at rest and false otherwise.
    * @note You must have called enableInterrupts() to ensure that the cached
    *   state is updated. An exception will be thrown if interrupts are not
    *   enabled.
    */
  bool atRestCached() const;

  /// Auxiliary exception class to be thrown when reading the cached state with disabled interrupts.
  class InterruptsAreDisabled : public std::runtime_error {
    public:
      /// Fill-in the exception message.
      InterruptsAreDisabled() : std::runtime_error("A call to Switch::atRestCached() was performed, but interrupts are disabled.") {}
  };

private:
  const int pin_; ///< Pin the switch is connected to.
  const int at_rest_; ///< Value that should be read on the pin when the switch is at rest.
  const unsigned int debounce_us_; ///< Microseconds to wait for debounce purposes.
  unsigned int last_tick_; ///< Last time an interrupt was served (used for debouncing).
  bool at_rest_now_; ///< Tells if the switch is now at rest.
  bool has_triggered_; ///< Tells if the switch has been activated.
  bool with_interrupts_; ///< If true, interrupts are being used.
  std::function<void(void)> callback_; ///< A custom callback that can be executed when the switch is activated.

  /// Internal callback to be executed everytime the switch changes state.
  /** @param gpio the pin that just changed its level.
    * @param level the current pin level.
    * @param tick time elapsed since boot (in microseconds). Note that it
    *   overflows every ~72 minutes (technically, every 2^32 microseconds).
    *   Remember that, due to integer arithmetics, it is still safe to get
    *   elasped times doing `tick_now-tick_previous` even if `tick_now`
    *   overflowed.
    * @todo Perhaps remove the internal pin check that throws an exception?
    */
  void onChange(
    int gpio,
    int level,
    unsigned int tick
  );

  /// Static wrapper to call onChange.
  /** This method is required since the GPIO library allows to execute a
    * callback when pin levels change, but such a callback must be provided
    * as a function pointer.
    * @param gpio the pin that just changed its level.
    * @param level the current pin level.
    * @param tick time elapsed since boot (in microseconds). Note that it
    *   overflows every ~72 minutes (technically, every 2^32 microseconds).
    *   Remember that, due to integer arithmetics, it is still safe to get
    *   elasped times doing `tick_now-tick_previous` even if `tick_now`
    *   overflowed.
    * @param THIS a pointer to an Encoder. Named `THIS` simply because it
    *   tries to mimic the `this` pointer.
    */
  static void onChangeStatic(
    int gpio,
    int level,
    unsigned int tick,
    void* THIS
  );
};

}
