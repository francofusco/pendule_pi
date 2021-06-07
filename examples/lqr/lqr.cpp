#include <pendule_pi/pendule_cpp.hpp>
#include <csignal>
#include <thread>
#include <cmath>


bool ok{true};

void sigintHandler(int signo) {
  ok = false;
}


double normalize(double angle) {
  while(angle > M_PI)
    angle -= 2*M_PI;
  while(angle < -M_PI)
    angle += 2*M_PI;
  return angle;
}


int main() {
  std::signal(SIGINT, sigintHandler);

  if(argc != 2) {
    std::cout << "Usage: " << argv[0] << " host-name" << std::endl;
    return EXIT_FAILURE;
  }

  pendule_pi::PenduleCpp pendulum(
    argv[1],
    pendule_pi::PenduleCpp::DEFAULT_STATE_PORT,
    pendule_pi::PenduleCpp::DEFAULT_COMMAND_PORT,
    5
  );

  int pwm = 0;
  const double MAX_ANGLE = 0.1;
  const double kp = -127.45880662905581;
  const double kpd = -822.638944546691;
  const double kt = 2234.654627319883;
  const double ktd = 437.1177135919267;

  while(ok) {
    pendulum.readState(pendulum.BLOCKING);
    auto et = normalize(pendulum.angle()-M_PI);
    if(std::fabs(et) < MAX_ANGLE)
      pwm = - kp * pendulum.position() - kt * et - kpd * pendulum.linvel() - ktd * pendulum.angvel();
    else
      pwm = 0;
    pendulum.sendCommand(pwm);
  }

  pendulum.sendCommand(0);

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  return EXIT_SUCCESS;
}
