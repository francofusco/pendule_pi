#include <pendule_pi/config.hpp>
#include <functional>

/// Simple argument parser that executes actions depending on the command-line parameters.
class SimpleArgParser {
public:
  typedef std::function<bool(const std::vector<std::string>& args, std::string& err)> action_t;

  SimpleArgParser();

  /// Add an argument with the corresponding action (function) to be executed.
  /** @param arg the argument that should trigger the action.
    * @param action the function to be executed when arg is given. Actions take
    *   two parameters and return a boolean. The first parameter is a vector of
    *   action-specific arguments, ie, all arguments given from the command
    *   line minus the first (program name) and second (command name) ones.
    *   The returned boolean should be true if the action was completed, and
    *   false if something went wrong. The second parameter of the action is
    *   a reference to a string that can be used to explain what went wrong.
    * @param help information about the argument and action, to be given to the
    *   user when the "help" command is used or when the action fails.
    */
  void addArgument(
    const std::string& arg,
    action_t action,
    const std::string& help
  );

  bool help(const std::vector<std::string>& args, std::string& err) const;

private:
  std::map<std::string,action_t> actions_;
  std::map<std::string,action_t> helps_;
};



int main(int argc, char** argv) {
  namespace pp = pendule_pi;



  const std::string cmd =

  try {
    // Let the token manage the pigpio library!
    pigpio::ActivationToken token;
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return EXIT_SUCCESS;
}


SimpleArgParser::SimpleArgParser() {
  // Wrap over the action
  auto help_action = [&](
    const std::vector<std::string>& args,
    std::string& err)
  {
    return this->help(args,err);
  };
  // Explain how to invoke this command
  auto help_msg = "Print usage messages on the console. It optionally accepts "
    "an argument, in which case help for that specific command will be printed.";
  // Add different arguments to provide help
  addArgument("help", help_action, help_msg);
  addArgument("-h", help_action, help_msg);
  addArgument("--help", help_action, help_msg);
}


void SimpleArgParser::addArgument(
  const std::string& arg,
  action_t action,
  const std::string& help
)
{
  if(actions_.find(arg) != actions_.end()) {
    throw std::runtime_error("Argument " + arg + " has been already added.");
  }
  actions_.insert({arg, action});
  helps_.insert({arg, help});
}
