#include <piksel/baseapp.hpp>
#include <pendule_pi/pendule_cpp_server.hpp>

class Simulator : public piksel::BaseApp {
public:
  Simulator();
  void setup() override;
  void draw(piksel::Graphics& g) override;

private:
  double time_{0};
  double position_{0};
  double angle_{0};
  double linvel_{0};
  double angvel_{0};
  const double dt_{0.02};
  const double MAX_POS{0.8};
  const double ROD_LEN{0.3};
  double ma, Ia, mua, fva, tva, g, maIa, Iafva, mag, matva;
  pendule_pi::PenduleCppServer tcp;

  inline double px(double x) { return width * (x+MAX_POS) / (2*MAX_POS); }
  inline double py(double y) { return (height+width)/2 - px(y); }

  void simulate(double F=0.0);
};


int main(int argc, char** argv) {
  Simulator sim;
  sim.start();
  return EXIT_SUCCESS;
}


Simulator::Simulator()
: piksel::BaseApp(640, 480)
{
  ma = 16.36107771481117;
  Ia = 0.29504025711512755;
  mua = 0.6546880349055427;
  fva = 403.0267679493358;
  tva = 0.02537703984258163;
  g = 9.806;
  maIa = ma*Ia;
  Iafva = Ia*fva;
  mag = ma*g;
  matva = ma*tva;
}


void Simulator::setup() {

}


void Simulator::draw(piksel::Graphics& g) {
  // g.background(glm::vec4(0, 0, 0, 1));
  g.background(glm::vec4(0.3, 0.3, 0.3, 1));

  tcp.readCommand(0);
  auto pwm = tcp.missedCommands() > 20 ? 0 : tcp.command();
  simulate(pwm);
  tcp.sendState(
    time_,
    position_,
    angle_,
    linvel_,
    angvel_
  );

  // // TEMP
  // static double t = 0;
  // position_ = 0.7*MAX_POS*std::sin(t);
  // angle_ = 2*t;
  // t += 0.01;

  auto cx = position_ + ROD_LEN * std::sin(angle_);
  auto cy = - ROD_LEN * std::cos(angle_);

  g.strokeWeight(5);
  g.line(px(position_), py(0), px(cx), py(cy));

  g.rectMode(piksel::DrawMode::CENTER);
  g.rect(px(position_), height/2, 30, 20);

  g.ellipse(px(cx), py(cy), 50, 50);

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
    auto cth = mua*std::cos(th);
    auto sth = mua*std::sin(th);
    auto thd2 = thd*thd;
    auto f1 = thd2*sth - fva*pd;
    auto f2 = -g*sth - tva*thd;
    auto uf1 = pwm + f1;
    auto num_p = Ia*uf1 - cth*f2;
    auto num_th = -cth*uf1 + ma*f2;
    auto dinv = 1/(maIa - cth*cth);
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
