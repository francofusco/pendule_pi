#include <pendule_pi/config.hpp>
#include <pendule_pi/pigpio.hpp>
#include <functional>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>

namespace pp = pendule_pi;


/// Simple argument parser that executes actions depending on the command-line parameters.
class SimpleArgParser {
public:
  typedef std::function<bool(const std::vector<std::string>& args, std::string& err)> action_t;

  /// Initializes the parser with help actions only.
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
    *   user when the "help" command is used or when the action fails. Helps are
    *   printed in the format "program_name command args\n description".
    *   The name of the program and the command are printed by SimpleArgParser,
    *   and thus the help messags should already start with the arguments and/or
    *   command description. To distinguish between arguments and description,
    *   insert the sequence ||| (if missing, no parameters are assumed by
    *   default).
    */
  void addArgument(
    const std::string& arg,
    action_t action,
    const std::string& help
  );

  /// Parse command line arguments and execute actions.
  bool parse(const std::vector<std::string>& args);

  /// Parse command line arguments and execute actions.
  bool parse(int argc, char** argv);

  /// Action that prints help messages.
  bool help(const std::vector<std::string>& args, std::string& err) const;

private:
  static std::string pretty(std::string msg);
  std::string program_name_; ///< Name of the executable.
  std::map<std::string,action_t> actions_; ///< Actions corresponding to each argument.
  std::map<std::string,std::string> helps_; ///< Help messages for each argument.
};


/// Action to print the YAML template configuration on the consol or save it to a file.
bool templateConfig(
  const std::vector<std::string>& args,
  std::string& err
);


/// Action to check if switches are properly setup.
bool testSwitches(
  const std::vector<std::string>& args,
  std::string& err
);


/// Action to check if encoders are properly setup.
bool testEncoders(
  const std::vector<std::string>& args,
  std::string& err
);


/// Action to check if motor is properly setup.
bool testMotor(
  const std::vector<std::string>& args,
  std::string& err
);


/// Action to evaluate the length of the rail in steps.
bool measureRail(
  const std::vector<std::string>& args,
  std::string& err
);


int main(int argc, char** argv) {

  SimpleArgParser parser;

  // Create a configuration file.
  parser.addArgument(
    "template",
    templateConfig,
    "[print]|[dump file]|||If 'print', a template configuration file will be "
    "printed on the console. If 'dump', the template will be written into the "
    "given file."
  );

  // Check that the switches have been properly configured.
  parser.addArgument(
    "test-switches",
    testSwitches,
    "config-file|||Allows to test if the switches are properly setup, by "
    "parsing the given configuration file to setup and monitor the state of "
    "the two components. Their state will be print on the console until you "
    "exit the assistant."
  );

  // Check that the encoders have been properly configured.
  parser.addArgument(
    "test-encoders",
    testEncoders,
    "config-file|||Allows to test if the encoders are properly setup, by "
    "parsing the given configuration file to setup the two components and print "
    "on the console their steps until you exit the assistant."
  );

  // Check that the motor has been properly configured.
  parser.addArgument(
    "test-motor",
    testMotor,
    "config-file pwm|||Allows to test if the motor is properly setup, by "
    "parsing the given configuration file and sending the specified pwm signal. "
    "!!! WARNING !!! use this command with much care, since the switches are "
    "not monitored here! The main purpose of this command is to test if the "
    "motor spins properly when disconnected, or to move the base away from a "
    "switch when a crash happens."
  );

  // Measure the the length in encoder steps of the rail.
  parser.addArgument(
    "measure-rail",
    measureRail,
    "config-file pwm|||Allows to measure the length of the rail in encoders steps."
    "Such number corresponds to the number of steps that the position encoder "
    "reads while going from one side to the other of the rail, using the "
    "switches to detect when one end has been reached. You need to provide the "
    "path to the configuration file (to setup GPIO connections with the "
    "hardware) and a pwm value that can be used to move the pendulum around. "
    "Lower values should lead to more accurate measures, but longer runtimes."
  );

  try {
    // Let the token manage the pigpio library!
    pigpio::ActivationToken token;
    // Parse the command line arguments and execute the required action.
    parser.parse(argc, argv);
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return EXIT_SUCCESS;
}


std::string SimpleArgParser::pretty(
  std::string msg
)
{
  std::string out;
  const std::string nl = "\n    ";
  const auto MAX_LEN = 60;

  // Part 1: Look for the '|||' pattern.
  auto pos = msg.find("|||");
  if(pos != std::string::npos) {
    // pattern found, extract the arguments and get rid of the pattern
    out += msg.substr(0,pos);
    msg = msg.substr(pos+3);
  }

  // Make sure that the message is not empty now
  if(msg.size() == 0) {
    return out;
  }

  // Part 2: Get rid of initial spaces and look for the first non-space char.
  int start = 0;
  while(true) {
    // Ensure there is at least one word
    if(start >= msg.size())
      return out;
    // Start now points to the first non-space char
    if(msg[start] != ' ')
      break;
    start++;
  }

  // Part 3: Split the message into parts.
  std::vector<std::string> parts;
  int stop = start+1;
  while(start < msg.size()) {
    // If we reached the end of the message or a space, we can extract a word.
    if(stop == msg.size() || msg[stop] == ' ') {
      // Avoid extracting empty chars due to consecutive spaces.
      if(start != stop) {
        auto w = msg.substr(start,stop-start);
        parts.push_back(w);
      }
      // Skip the current char, since it should be a space.
      start = stop+1;
    }
    // Check next char in the message.
    stop++;
  }

  // Part 4: Put words together.
  out += nl;
  std::string line = parts[0];
  parts.erase(parts.begin());

  for(const auto& w : parts) {
    if(line.size() + w.size() + 1 > MAX_LEN) {
      // If the current word (plus a preceding space) would make the current
      // line too long, write the current line content and go to a new one.
      out += line + nl;
      line = w;
    }
    else {
      // The current line plus the new word is not too long!
      line += " " + w;
    }
  }

  // make sure to write whatever remains from the last line.
  return out + line;
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
  addArgument("-h", help_action, "Like 'help'");
  addArgument("--help", help_action, "Like 'help'");
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


bool SimpleArgParser::parse(const std::vector<std::string>& args) {
  using std::cout;
  using std::endl;
  // Make sure to assign a name to the program
  program_name_ = args.size()>0 ? args[0] : std::string("ANONYMOUS_PROGRAM");

  // String to get error messages from actions.
  std::string err;

  if(args.size() < 2) {
    // No command given: run the "help" action and exit.
    cout << "No command given." << endl;
    help({}, err);
    return false;
  }
  else if(helps_.find(args[1]) == helps_.end()) {
    // Unkown command given: run the "help" action and exit.
    cout << "Invalid command '" << args[1] << "' given." << endl;
    help({}, err);
    return false;
  }
  else {
    // Run the given command and print an error if the action fails.
    std::vector<std::string> cmd_args(args.begin()+2, args.end());
    auto retv = actions_[args[1]](cmd_args, err);
    if(!retv) {
      cout << "Failed to execute command '" << args[1] << "'." << endl;
      cout << "Error: " << err << endl;
      help({args[1]}, err);
    }
    return retv;
  }
  // This should not be reachable!
  cout << "FATAL ERROR: this point of the function SimpleArgParser::parse "
    "should be impossible to reach. Please, contact the maintainer!" << endl;
  return false;
}


bool SimpleArgParser::parse(int argc, char** argv) {
  std::vector<std::string> args;
  args.reserve(argc);
  for(int i=0; i<argc; i++)
    args.push_back(std::string(argv[i]));
  return parse(args);
}


bool SimpleArgParser::help(
  const std::vector<std::string>& args,
  std::string& err
) const
{
  using std::cout;
  using std::endl;
  cout << "Usage:" << endl;

  if(args.size() == 1 && helps_.find(args[0]) != helps_.end()) {
    cout << "  " << program_name_ << " " << args[0] << " " << pretty(helps_.at(args[0])) << endl;
  }
  else {
    cout << "  " << program_name_ << " action [args]" << endl;
    cout << endl;
    cout << "Available commands:" << endl;
    for(const auto& arg : helps_) {
      cout << endl;
      cout << "  " << arg.first << " " << pretty(arg.second) << endl;
    }
  }

  return true;
}


bool templateConfig(
  const std::vector<std::string>& args,
  std::string& err
)
{
  if(args.size() == 0) {
    err = "Not enough arguments.";
    return false;
  }

  if(args[0] == "print") {
    std::cout << pendule_pi::Config::getTemplate();
    return true;
  }

  if(args[0] == "dump") {
    if(args.size() < 2) {
      err = "Missing file name.";
      return false;
    }
    auto retv = pendule_pi::Config::dumpTemplate(args[1]);
    if(!retv) {
      err = "Failed to dump the configuration into '" + args[1] + "'.";
    }
    return retv;
  }

  err = "Unkown argument '" + args[0] + "'";
  return false;
}


bool testSwitches(
  const std::vector<std::string>& args,
  std::string& err
)
{
  if(args.size() == 0) {
    err = "Missing configuration file.";
    return false;
  }

  pp::Config config(args[0]);
  auto left_switch = config.getLeftSwitch();
  auto right_switch = config.getRightSwitch();
  left_switch->enableInterrupts();
  right_switch->enableInterrupts();

  std::cout << "LEFT SWITCH     RIGHT SWITCH" << std::endl;
              //  AT REST         AT REST
              // ACTIVATED       ACTIVATED

  while(true) {
    std::cout
      << "\r"
      << (left_switch->atRestCached() ? "  AT REST  " : " ACTIVATED ")
      << "     "
      << (right_switch->atRestCached() ? "  AT REST   " : " ACTIVATED  ")
      << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  return true;
}


bool testEncoders(
  const std::vector<std::string>& args,
  std::string& err
)
{
  if(args.size() == 0) {
    err = "Missing configuration file.";
    return false;
  }

  pp::Config config(args[0]);
  auto position_encoder = config.getPositionEncoder();
  auto angle_encoder = config.getAngleEncoder();

  std::cout << " POSITION       ANGLE " << std::endl;
              //+00000000    +00000000
  std::cout << std::setfill('0') << std::internal << std::showpos;

  while(true) {
    std::cout
      << "\r"
      << std::setw(8) << position_encoder->steps()
      << "     "
      << std::setw(8) << angle_encoder->steps()
      << std::flush;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  return true;
}


bool testMotor(
  const std::vector<std::string>& args,
  std::string& err
)
{
  if(args.size() == 0) {
    err = "Missing configuration file and pwm value.";
    return false;
  }
  else if(args.size() == 1) {
    err = "Missing pwm value.";
    return false;
  }

  pp::Config config(args[0]);
  auto motor = config.getMotor();

  int pwm = std::stoi(args[1]);
  std::cout << "Sending PWM " << pwm << std::endl;
  motor->setPWM(pwm);

  // enter into an infinite loop!
  while(true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  }

  return true;
}


bool measureRail(
  const std::vector<std::string>& args,
  std::string& err
)
{
  if(args.size() == 0) {
    err = "Missing configuration file and pwm value.";
    return false;
  }
  else if(args.size() == 1) {
    err = "Missing pwm value.";
    return false;
  }

  pp::Config config(args[0]);
  auto motor = config.getMotor();
  auto left_switch = config.getLeftSwitch();
  auto right_switch = config.getRightSwitch();
  auto position_encoder = config.getPositionEncoder();

  if(!left_switch->atRest()) {
    err = "Left switch not at rest";
    return false;
  }

  if(!right_switch->atRest()) {
    err = "Right switch not at rest";
    return false;
  }

  std::vector<int> switches;
  int rail_len=0,rail_len_variance=0;
  switches.reserve(200);

  auto updateMeasure = [&](){
    int steps = position_encoder->steps();
    motor->setPWM(-motor->getPWM());
    switches.push_back(steps);
    if(switches.size() < 2)
      return;

    double avg = 0, stddev = 0;
    for(unsigned int i=0; i<switches.size()-1; i++) {
      avg += std::abs(switches[i]-switches[i+1]);
    }
    avg = avg / (switches.size()-1);
    for(unsigned int i=0; i<switches.size()-1; i++) {
      stddev += std::pow(avg-std::abs(switches[i]-switches[i+1]), 2);
    }
    stddev = std::sqrt(stddev / (switches.size()-1));

    // reset values
    rail_len = static_cast<int>(std::round(avg));
    rail_len_variance = static_cast<int>(std::round(stddev));
  };

  left_switch->enableInterrupts(updateMeasure);
  right_switch->enableInterrupts(updateMeasure);
  motor->setPWM(std::stoi(args[1]));

  std::cout << std::setfill('0') << std::internal;

  // enter into an infinite loop!
  while(true) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::cout << "\rLEN: " << rail_len << "   STD.DEV.: " << rail_len_variance << "    " << std::flush;
  }

  return true;
}
