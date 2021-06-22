#include <pendule_pi/pendule_cpp_client.hpp>
#include <sstream>
#include <thread>

namespace pendule_pi {

PenduleCppClient::PenduleCppClient(
  int wait
)
: PenduleCppClient(
    DEFAULT_HOST,
    DEFAULT_STATE_PORT,
    DEFAULT_COMMAND_PORT,
    wait
  )
{ }


PenduleCppClient::PenduleCppClient(
  const std::string& host,
  const std::string& state_port,
  const std::string& command_port,
  int wait
)
{
  // Connect to the sockets to exchange data with the low-level interface.
  context_ = std::make_unique<zmqpp::context>();
  state_sub_ = std::make_unique<zmqpp::socket>(*context_, zmqpp::socket_type::subscribe);
  state_sub_->set(zmqpp::socket_option::conflate, 1);
  state_sub_->connect("tcp://" + host + ":" + state_port);
  state_sub_->subscribe("");
  command_pub_ = std::make_unique<zmqpp::socket>(*context_, zmqpp::socket_type::publish);
  command_pub_->connect("tcp://" + host + ":" + command_port);

  // TODO: wait for connection to be established.
  // Wait for the low-level interface to be up and running.
  if(wait <= 0) {
    // Wait indefinitely for the first message.
    readState(BLOCKING);
  }
  else {
    // Wait for the given amount of time.
    int elapsed = 0;
    while(elapsed < wait) {
      // Wait one second.
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      // Check if it is possible to read a message.
      if(readState(NON_BLOCKING)) {
        // A message was received: exit!
        break;
      }
      elapsed += 1;
      // Did we exit due to a received message? If not, throw!
      if(elapsed >= wait) {
        throw std::runtime_error("PenduleCppClient: failed to establish a "
          "connection with the low-level interface within the allotted time.");
      }
    }
  }
}


PenduleCppClient::~PenduleCppClient()
{
  state_sub_.reset();
  command_pub_.reset();
  context_.reset();
}


bool PenduleCppClient::readState(
  bool blocking
)
{
  // Message to be received.
  zmqpp::message msg;
  // Try receiving a message.
  if(!state_sub_->receive(msg, !blocking)) {
    return false;
  }
  // Split the string message into parts. Each part should be a double.
  std::string msg_str;
  msg >> msg_str;
  std::istringstream iss(msg_str);
  iss >> time_ >> position_ >> angle_ >> linvel_ >> angvel_;
  // State read successfully.
  return true;
}


void PenduleCppClient::sendCommand(int pwm) {
  // Create the message to be sent.
  zmqpp::message msg;
  msg << std::to_string(pwm);
  // Send the PWM to the low-level interface.
  command_pub_->send(msg);
}

} // namespace pendule_pi
