#include <pendule_pi/pendule_cpp.hpp>
#include <csignal>
#include <thread>


bool ok{true};

void sigintHandler(int signo) {
  ok = false;
}


int main(int argc, char** argv) {
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

  int pwm = 25;
  double switch_pos = 0.2;

  while(ok) {
    pendulum.readState(pendulum.BLOCKING);
    if((pendulum.position() > switch_pos && pwm > 0) || (pendulum.position() < -switch_pos && pwm < 0))
      pwm *= -1;
    pendulum.sendCommand(pwm);
  }

  pendulum.sendCommand(0);

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));

  return EXIT_SUCCESS;
}
