/** @file encoder.hpp
  * @brief Header file for the Encoder class.
  */
#pragma once

namespace pendule_pi {

/// Class that handles a simple rotatory encoder.
class Encoder {
public:
  /// Constructor, requires the 2 signal pins of the encoder.
  /** TODO
    */
  Encoder(int gpioA, int gpioB);

  /// Destructor.
  /** TODO
    */
  virtual ~Encoder();

  /// Access the current step counter.
  inline const int& steps() const { return steps_; }

private:
  int pin_a_; ///< Pin the first wire of the encoder is connected to.
  int pin_b_; ///< Pin the second wire of the encoder is connected to.
  int level_a_; ///< Current voltage level on pin_a_.
  int level_b_; ///< Current voltage level on pin_b_.
  int last_triggered_; ///< The last pin that triggered a GPIO hardware interrupt.
  int steps_; ///< Current number of encoder steps.

  /// Function called whenever one of the pins changes level.
  /** TODO
    */
  void pulse(
    int gpio,
    int level,
    unsigned int tick
  );

  /// Static wrapper to call pulse.
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
  static void pulseStatic(
    int gpio,
    int level,
    unsigned int tick,
    void* THIS
  );

};

}
