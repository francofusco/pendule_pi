/** @file encoder.hpp
  * @brief Header file for the Encoder class.
  */
#pragma once

#include <vector>
#include <functional>

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

  /// Puts the two GPIO pins into high impedance mode.
  virtual ~Encoder();

  /// Access the current step counter.
  inline const int& steps() const { return steps_; }
  /// Access the current motor direction.
  inline const int& direction() const { return direction_; }

  /// Add callbacks to be executed when reaching safety thresholds.
  /** @param lower_threshold lower safety threshold below which lower_cb should
    *   be called.
    * @param upper_threshold upper safety threshold beyond which upper_cb
    *   should be called.
    * @param lower_cb function to be executed when the position becomes smaller
    *   or equal to lower_threshold. It can be nullptr, in which case no
    *   callback is executed.
    * @param upper_cb function to be executed when the position becomes greater
    *   or equal to upper_threshold. It can be nullptr, in which case no
    *   callback is executed.
    * @warning The thresholds upper_threshold and lower_threshold are
    *   absolutely arbitrary. As an example, there is no need for the lower
    *   threshold to be smaller than the upper one. But be careful: with great
    *   power comes great responsibility!
    */
  void setSafetyCallbacks(
    int lower_threshold,
    int upper_threshold,
    std::function<void(void)> lower_cb,
    std::function<void(void)> upper_cb
  );

  /// Add callbacks to be executed when reaching safety thresholds.
  /** Like
    * setSafetyCallbacks(int,int,std::function<void(void)>,std::function<void(void)>)
    * but using the same callback for both the lower and the upper thresholds.
    * @param lower_threshold lower safety threshold below which cb should be
    *   called.
    * @param upper_threshold upper safety threshold beyond which cb should be
    *   called.
    * @param cb function to be executed when the position becomes either smaller
    *   (or equal) to lower_threshold or  greater (or equal) to upper_threshold.
    *   It can be nullptr, in which case no callback is executed.
    */
  inline void setSafetyCallbacks(
    int lower_threshold,
    int upper_threshold,
    std::function<void(void)> cb
  )
  {
    setSafetyCallbacks(lower_threshold, upper_threshold, cb, cb);
  }

private:
  /// "Table" that allows to tell the direction of rotation.
  /** The encoder can be seen as a simple state machine in which the state
    * is given by the pair of voltage levels `(a,b)`. For each state,
    * there are exactly two possible transitions towards other states.
    * It is thus possible to define a simple lookup-table in which one stores
    * the "departing" (or past) state as well as the "arrival" (or current)
    * one and associates this pair to a direction. Such table is shown here:
    *
    *  a' | b' | a | b | dir
    * ----|----|---|---|-----
    *  0  | 0  | 0 | 0 |  0
    *  0  | 0  | 0 | 1 | +1
    *  0  | 0  | 1 | 0 | -1
    *  0  | 0  | 1 | 1 |  0
    *  0  | 1  | 0 | 0 | -1
    *  0  | 1  | 0 | 1 |  0
    *  0  | 1  | 1 | 0 |  0
    *  0  | 1  | 1 | 1 | +1
    *  1  | 0  | 0 | 0 | +1
    *  1  | 0  | 0 | 1 |  0
    *  1  | 0  | 1 | 0 |  0
    *  1  | 0  | 1 | 1 | -1
    *  1  | 1  | 0 | 0 |  0
    *  1  | 1  | 0 | 1 | -1
    *  1  | 1  | 1 | 0 | +1
    *  1  | 1  | 1 | 1 |  0
    *
    * The values `a'` and `b'` represent past states, while `a` and `b` are the
    * current ones. As an example, if at any moment the encoder changes from
    * `(0,1)` to `(1,1)`, then it means that it rotated of one step in the
    * positive direction. Impossible transitions (such as `(0,0)->(1,1)`) are
    * given a null value so that they do not produce any effect.
    *
    * The lookup table above is stored in this variable, in the form of a
    * vector. To read it, it is sufficient to pass as index the number obtained
    * by reading the quadruple of pin values as a binary number. As an example,
    * consider the case in which the encoder transitions from `(0,1)` to
    * `(0,0)`. The four digits altogether form the binary number `0100`,
    * corresponding to 4. The direction corresponding to this transition is
    * thus available at `ENCODER_TABLE[4]`.
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
  int direction_; ///< Current rotation direction.
  int lower_threshold_; ///< Lower threshold below which lower_cb_ should be executed.
  int upper_threshold_; ///< Upper threshold beyond which upper_cb_ should be executed.
  std::function<void(void)> lower_cb_; ///< Callback to be executed whenver the position becomes less than lower_threshold_.
  std::function<void(void)> upper_cb_; ///< Callback to be executed whenver the position becomes more than upper_threshold_.

  /// Function called whenever one of the pins changes level.
  /** @param gpio the pin that just changed its level.
    * @param level the current pin level.
    * @param tick time elapsed since boot (in microseconds). Note that it
    *   overflows every ~72 minutes (technically, every 2^32 microseconds).
    *   Remember that, due to integer arithmetics, it is still safe to get
    *   elasped times doing `tick_now-tick_previous` even if `tick_now`
    *   overflowed.
    * @todo Perhaps remove the internal pin check that throws an exception?
    * @todo Perhaps change ENCODER_TABLE.at(i) for ENCODER_TABLE[i]?
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
