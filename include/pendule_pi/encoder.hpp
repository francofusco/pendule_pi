/** @file encoder.hpp
  * @brief Header file for the Encoder class.
  */
#pragma once

#include <vector>

namespace pendule_pi {

/// Class that handles a simple two-phases rotatory encoder.
class Encoder {
public:
  /// Constructor, performs the setup of the gpio.
  /** This class allows to handle a rotatory encoder, assuming that it uses two
    * phases in phase quadrature.
    * @param gpioA the pin connected to the first phase.
    * @param gpioB the pin connected to the second phase.
    */
  Encoder(
    int gpioA,
    int gpioB
  );

  /// Destructor.
  virtual ~Encoder();

  /// Access the current step counter.
  inline const int& steps() const { return steps_; }

private:
  /// "Table" that allows to tell the direction of rotation.
  /** The table is the following:
    * a'b'a b dir
    * -------------
    * 0 0 0 0  0
    * 0 0 0 1 +1
    * 0 0 1 0 -1
    * 0 0 1 1  0
    * 0 1 0 0 -1
    * 0 1 0 1  0
    * 0 1 1 0  0
    * 0 1 1 1 +1
    * 1 0 0 0 +1
    * 1 0 0 1  0
    * 1 0 1 0  0
    * 1 0 1 1 -1
    * 1 1 0 0  0
    * 1 1 0 1 -1
    * 1 1 1 0 +1
    * 1 1 1 1  0
    *
    * Note that the direction is 0 whenver the (a,b) pair is equal to (a',b')
    * or opposite to it.
    * @todo Formatting the table in the proper way!
    */
  static const std::vector<int> ENCODER_TABLE;

  /// Convert pin levels into the correct index to be passed to ENCODER_TABLE.
  /** @param a current level of the first phase.
    * @param b current level of the second phase.
    * @param past_a past level of the first phase.
    * @param past_b past level of the second phase.
    * @return an index to be used inside ENCODER_TABLE which tells in which
    *   direction the encoder is rotating.
    */
  static int encode(
    bool a,
    bool b,
    bool past_a,
    bool past_b
  );

  int pin_a_; ///< Pin the first wire of the encoder is connected to.
  int pin_b_; ///< Pin the second wire of the encoder is connected to.
  bool a_current_; ///< Current voltage level on pin_a_.
  bool b_current_; ///< Current voltage level on pin_b_.
  bool a_past_; ///< Past voltage level on pin_a_.
  bool b_past_; ///< Past voltage level on pin_b_.
  int steps_; ///< Current number of encoder steps.


  /// Function called whenever one of the pins changes level.
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
