#pragma once
#include <cmath>


/// Simple function that returns 0 if `abs(val) < zero` and `val` otherwise.
int robustZero(int val, int zero) {
  if(-zero < val && val < zero)
    return 0;
  else
    return val;
}

/// Piecewise linear function that is 0 in a neighborhood of 0.
/** This function is defined as:
  * \f[
  *   c(x) =
  *   \begin{cases}
  *     x + z & (x \leq -z) \\
  *     0     & (-z < x < z) \\
  *     x - z & (z \leq x)
  *   \end{cases}
  * \f]
  */
int clip(int val, int zero) {
  if(val > zero)
    return val - zero;
  else if(val < -zero)
    return val + zero;
  else
    return 0;
}

/// Simple sign function.
/** \f[
  *   sign(x) =
  *   \begin{cases}
  *     -1 & (x < 0)
  *      1 & (x \geq 0)
  *   \end{cases}
  * \f]
  */
inline double sign(double x) {
  return x>=0 ? 1.0 : -1.0;
}

/// Modular shift in the unit circle
/** Values that lie outside the range \f$\mathbb{C}=\left[-\pi,\pi\right)\f$
  * will be shifted by \f$2 k \pi\f$ (\f$ k \in \mathbb{Z} \f$) so that they lie
  * within \f$\mathbb{C}\f$.
  */
double shiftInCircle(double x)
{
  double v = std::fmod(x, 2*M_PI);
  if(v >= M_PI)
    v -= 2*M_PI;
  if(v < -M_PI)
    v += 2*M_PI;
  return v;
}


/// Return a list of evenly spaced values.
/** Returns the list of all values in the form `start + k * step` that are
  * smaller than (or equal to) `stop`.
  *
  * As an example, `range(1.2, 2.1, 0.2)` would return a vector that is
  * equivalent to `{1.2, 1.4, 1.6, 1.8, 2.0}`.
  */
std::vector<int> range(int start, int stop, int step) {
  std::vector<int> vec;
  for(int i=start; i<=stop; i+=step)
    vec.push_back(i);
  return vec;
}
