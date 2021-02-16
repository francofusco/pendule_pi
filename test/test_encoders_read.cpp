#include "pendule_pi/pigpio.hpp"
#include "pendule_pi/encoder.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <thread>


void printInt(int val, int digits) {
  std::string sig = val >= 0 ? "+" : "-";
  val = std::abs(val);
  std::cout << sig << std::setw(digits) << std::setfill('0') << val;
}

int main(int argc, char** argv) {
  namespace pp = pendule_pi;
  try {
    // Let the token manage the pigpio library
    pigpio::ActivationToken token;
    // Create the encoder
    pp::Encoder encoder_linear(20, 21);
    pp::Encoder encoder_angular(19, 26);
    const int NDIGITS = 7;
    std::cout << "  LINEAR   ANGULAR" << std::endl;
    while(true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));
      std::cout << "\r";
      printInt(encoder_linear.steps(), NDIGITS);
      std::cout << "  ";
      printInt(encoder_angular.steps(), NDIGITS);
      std::cout << std::flush;
    }
  }
  catch(const pigpio::ActivationToken::PleaseStop&) { }
  catch(...) { throw; }

  return 0;
}
