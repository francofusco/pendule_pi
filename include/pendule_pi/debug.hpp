#pragma once
#include <iostream>

#ifdef PENDULE_PI_DEBUG_ENABLED
  #define PENDULE_PI_DBG(x) {std::cout << "\033[0;32m[DEBUG]\033[00m " << x << std::endl;}
#else
  #define PENDULE_PI_DBG(x) {};
#endif

#define PENDULE_PI_WRN(x) {std::cout << "\033[0;33m[WARN]\033[00m " << x << std::endl;}
