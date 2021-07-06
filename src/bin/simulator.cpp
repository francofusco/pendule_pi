#include <piksel/baseapp.hpp>
#include <yaml-cpp/yaml.h>
#include <pendule_pi/pendule_cpp_server.hpp>

class Simulator : public piksel::BaseApp {
public:
  Simulator(unsigned int width, unsigned int height);
  void setup() override;
  void draw(piksel::Graphics& g) override;

  void setDynamicParameters(
    double ma,
    double Ia,
    double mua,
    double fva,
    double tva
  );

  void setInitialConditions(
    double position,
    double angle,
    double linvel,
    double angvel
  );

  void setSoftLimit(double max_pos);

private:
  double time_{0};
  double position_{0};
  double angle_{0};
  double linvel_{0};
  double angvel_{0};
  const double dt_{0.02};
  double max_pos_{0.8};
  const double ROD_LEN{0.3};
  double ma_{16.36107771481117};
  double Ia_{0.29504025711512755};
  double mua_{0.6546880349055427};
  double fva_{403.0267679493358};
  double tva_{0.02537703984258163};
  static constexpr double g = 9.806;
  double maIa_;
  double Iafva_;
  double mag_;
  double matva_;
  pendule_pi::PenduleCppServer tcp_;
  std::chrono::time_point<std::chrono::system_clock> last_triggered_;


  inline double px(double x) const { return width * (x+max_pos_) / (2*max_pos_); }
  inline double py(double y) const { return (height+width)/2 - px(y); }
  void sleep();
  void simulate(double F=0.0);
};


Simulator::Simulator(
  unsigned int width,
  unsigned int height
)
: piksel::BaseApp(width, height)
{
  setDynamicParameters(ma_, Ia_, mua_, fva_, tva_);
}


void Simulator::setDynamicParameters(
  double ma,
  double Ia,
  double mua,
  double fva,
  double tva
)
{
  ma_ = ma;
  Ia_ = Ia;
  mua_ = mua;
  fva_ = fva;
  tva_ = tva;
  maIa_ = ma*Ia;
  Iafva_ = Ia*fva;
  mag_ = ma*g;
  matva_ = ma*tva;
}


void Simulator::setInitialConditions(
  double position,
  double angle,
  double linvel,
  double angvel
)
{
  position_ = position;
  angle_ = angle;
  linvel_ = linvel;
  angvel_ = angvel;
}


void Simulator::setSoftLimit(double max_pos) {
  if(max_pos <= 0)
    throw std::runtime_error("Parameter 'max_pos' passed to Simulator::setSoftLimit() must be positive.");
  max_pos_ = max_pos;
}


void Simulator::setup() {
  last_triggered_ = std::chrono::system_clock::now();
}


void Simulator::draw(piksel::Graphics& g) {

  // Time to sleep to enforce a maximum rate (not sure that it is needed!).
  auto elapsed = std::chrono::system_clock::now() - last_triggered_;
  auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
  auto period_ms = static_cast<int>(1e3*dt_);
  int to_sleep_ms = elapsed_ms < period_ms ? period_ms - elapsed_ms : 0;

  // Get command from socket.
  tcp_.readCommand(to_sleep_ms);
  // Update current time.
  last_triggered_ = std::chrono::system_clock::now();
  auto pwm = tcp_.missedCommands() > 20 ? 0 : tcp_.command();
  // Enforce soft limits.
  if((position_ > max_pos_ && pwm > 0) || (position_ < -max_pos_ && pwm < 0))
    pwm = 0;
  // Simulate the system given the current command.
  simulate(pwm);
  // Publish the new state.
  tcp_.sendState(
    time_,
    position_,
    angle_,
    linvel_,
    angvel_
  );

  // Draw the pendulum.
  g.background(glm::vec4(0.3, 0.3, 0.3, 1));
  auto cx = position_ + ROD_LEN * std::sin(angle_);
  auto cy = - ROD_LEN * std::cos(angle_);
  g.strokeWeight(5);
  g.line(px(position_), py(0), px(cx), py(cy));
  g.rectMode(piksel::DrawMode::CENTER);
  g.rect(px(position_), height/2, 30/max_pos_, 20/max_pos_);
  g.ellipse(px(cx), py(cy), 50/max_pos_, 50/max_pos_);
}


void Simulator::simulate(double pwm) {
  constexpr auto subdivisions = 10;
  const auto dt = dt_ / subdivisions;
  const auto dt2 = dt*dt;
  for(unsigned int i=0; i<subdivisions; i++) {
    // copy the state
    auto p = position_;
    auto th = angle_;
    auto pd = linvel_;
    auto thd = angvel_;
    // auxiliary variables
    auto cth = mua_*std::cos(th);
    auto sth = mua_*std::sin(th);
    auto thd2 = thd*thd;
    auto f1 = thd2*sth - fva_*pd;
    auto f2 = -g*sth - tva_*thd;
    auto uf1 = pwm + f1;
    auto num_p = Ia_*uf1 - cth*f2;
    auto num_th = -cth*uf1 + ma_*f2;
    auto dinv = 1/(maIa_ - cth*cth);
    // evaluate the acceleration
    auto pdd = num_p * dinv;
    auto thdd = num_th * dinv;
    // make one step
    position_ += dt*pd + 0.5*dt2*pdd;
    angle_ += dt*thd + 0.5*dt2*thdd;
    linvel_ += dt*pdd;
    angvel_ += dt*thdd;
  }
  time_ += dt_;
}


int main(int argc, char** argv) {
  // Provide some help instructions.
  if(argc >= 2 && (argv[1] == std::string("help") || argv[1] == std::string("-h") || argv[1] == std::string("--help")))
  {
    std::cout << "Usage: " << argv[0] << " configuration.yaml" << std::endl;
    return EXIT_SUCCESS;
  }

  // Load the configuration for this demo.
  YAML::Node yaml;
  if(argc > 1)
    yaml = YAML::LoadFile(argv[1]);

  // Create the simulator.
  const int DEFAULT_WIDTH = 800;
  const int DEFAULT_HEIGHT = static_cast<int>(0.75*800);
  Simulator sim(
    yaml["window_width"].as<int>(DEFAULT_WIDTH),
    yaml["window_height"].as<int>(DEFAULT_HEIGHT)
  );
  // Set dynamic parameters.
  sim.setDynamicParameters(
    yaml["dynamic_parameters"]["ma"].as<double>(16.36107771481117),
    yaml["dynamic_parameters"]["Ia"].as<double>(0.29504025711512755),
    yaml["dynamic_parameters"]["mua"].as<double>(0.6546880349055427),
    yaml["dynamic_parameters"]["fva"].as<double>(403.0267679493358),
    yaml["dynamic_parameters"]["tva"].as<double>(0.02537703984258163)
  );
  // Set initial conditions.
  sim.setInitialConditions(
    yaml["initial_state"]["position"].as<double>(0.0),
    yaml["initial_state"]["angle"].as<double>(0.0),
    yaml["initial_state"]["linear_velocity"].as<double>(0.0),
    yaml["initial_state"]["angular_velocity"].as<double>(0.0)
  );
  // Set maximum position limit if required.
  if(yaml["soft_limit"])
    sim.setSoftLimit(yaml["soft_limit"].as<double>());
  // Have fun!
  sim.start();
  return EXIT_SUCCESS;
}
