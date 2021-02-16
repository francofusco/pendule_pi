#include "pendule_pi/pendule.hpp"

namespace pendule_pi {

Pendule::Pendule(
  // params here!
)
: calibrated_(false)
{
  throw std::runtime_error("Pendule::Pendule not implemented");
}


void Pendule::calibrate()
{
  throw std::runtime_error("Pendule::calibrate not implemented");
}


void Pendule::update(
  double dt
)
{
  throw std::runtime_error("Pendule::update not implemented");
  // Get raw encoder reading
  double new_position = steps2meters(position_encoder_->steps());
  double new_angle = steps2radians(angle_encoder_->steps());
  // State estimation: TODO!
  linvel_ = (new_position-position_) / dt;
  angvel_ = (new_angle-angle_) / dt;
  position_ = new_position;
  angle_ = new_angle;
}

}
