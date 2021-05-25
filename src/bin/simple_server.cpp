#include <iostream>
#include <zmqpp/zmqpp.hpp>
#include <thread>

int main(int argc, char** argv) {
  int period_ms = 1000 * (argc < 2 ? 1.0 : std::atof(argv[1]));
  std::cout << "Main loop period: " << (period_ms / 1000.) << std::endl;

  zmqpp::context context;

  zmqpp::socket pub(context, zmqpp::socket_type::publish);
  pub.bind("tcp://*:10001");

  zmqpp::socket sub(context, zmqpp::socket_type::subscribe);
  sub.set(zmqpp::socket_option::conflate, 1);
  sub.bind("tcp://*:10002");
  sub.subscribe("");

  int pub_count = 0;

  while(true) {
    std::cout << "----------" << std::endl;
    std::string msg_str = "Sent by server " + std::to_string(++pub_count);
    zmqpp::message msg;
    msg << msg_str;
    pub.send(msg, true);
    std::cout << "Sent message: " << msg_str << std::endl;
    if(sub.receive(msg, true)) {
      std::string s;
      msg >> s;
      std::cout << "Received message: " << s << std::endl;
    }
    else {
      std::cout << "No message received" << std::endl;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(period_ms));
  }

  return EXIT_SUCCESS;
}
