#pragma once

#ifdef PENDULE_PI_DEBUG_ENABLED
  #include <iostream>
  #define PENDULE_PI_DBG(x) {std::cout << "\033[0;32m[DEBUG]\033[00m " << x << std::endl;}
#else
  #define PENDULE_PI_DBG(x) {};
#endif
