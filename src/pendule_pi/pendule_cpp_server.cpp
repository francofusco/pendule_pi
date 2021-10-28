#include <pendule_pi/pendule_cpp_server.hpp>
#include <sstream>
#include <thread>

namespace pendule_pi {

PenduleCppServer::PenduleCppServer()
: PenduleCppServer(
    DEFAULT_HOST,
    DEFAULT_STATE_PORT,
    DEFAULT_COMMAND_PORT
  )
{ }


PenduleCppServer::PenduleCppServer(
  const std::string& host,
  const std::string& state_port,
  const std::string& command_port
)
{
  // Connect to the sockets to exchange data with the low-level interface.
  context_ = std::make_unique<zmqpp::context>();
  state_pub_ = std::make_unique<zmqpp::socket>(*context_, zmqpp::socket_type::publish);
  state_pub_->bind("tcp://" + host + ":" + state_port);
  command_sub_ = std::make_unique<zmqpp::socket>(*context_, zmqpp::socket_type::subscribe);
  command_sub_->set(zmqpp::socket_option::conflate, 1);
  command_sub_->bind("tcp://" + host + ":" + command_port);
  command_sub_->subscribe("");
  command_poller_ = std::make_unique<zmqpp::poller>();
  command_poller_->add(*command_sub_, zmqpp::poller::poll_in);
}


PenduleCppServer::~PenduleCppServer()
{
  command_poller_.reset();
  command_sub_.reset();
  state_pub_.reset();
  context_.reset();
}


bool PenduleCppServer::readCommand(
  int wait_ms
)
{
  // Poll the socket if needed.
  if(wait_ms > 0 && !command_poller_->poll(wait_ms)) {
    missed_messages_++;
    return false;
  }
  // Message to be received.
  zmqpp::message msg;
  // Try receiving a message.
  if(!command_sub_->receive(msg, true)) {
    missed_messages_++;
    return false;
  }
  // Split the string message into parts. Each part should be a double.
  std::string msg_str;
  msg >> msg_str;
  std::istringstream iss(msg_str);
  iss >> pwm_;
  missed_messages_ = 0;
  // State read successfully.
  return true;
}


void PenduleCppServer::sendState(
  double time,
  double position,
  double angle,
  double linvel,
  double angvel
)
{
  // Create the message to be sent.
  zmqpp::message msg;
  msg << std::to_string(time) + " "
       + std::to_string(position) + " "
       + std::to_string(angle) + " "
       + std::to_string(linvel) + " "
       + std::to_string(angvel);
 // Send the state.
  state_pub_->send(msg, true);
}

} // namespace pendule_pi
