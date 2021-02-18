cmake_minimum_required(VERSION 3.15)
project(pendule_pi VERSION 1.0.0)
add_definitions(-std=c++17)


###################
# CONFIGURE BUILD #
###################
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -Wpedantic -Wextra -fexceptions")



#######################
# LOCATE DEPENDENCIES #
#######################

find_package(pigpio)


#####################
# BUILD SOURCE CODE #
#####################

# Define some colors for output formatting
string(ASCII 27 Esc)
set(ColourReset "${Esc}[m")
set(Yellow      "${Esc}[33m")
message(WARNING "${Yellow}TODO: Fix bug related to exceptions inside ISRs${ColourReset}")

if(${pigpio_FOUND})
  ################
  # MAIN LIBRARY #
  ################
  add_library(pendule_pi
    src/joystick.cpp
    src/pigpio.cpp
    src/switch.cpp
    src/motor.cpp
    src/encoder.cpp
    src/pendule.cpp
  )

  target_include_directories(pendule_pi PUBLIC
    include
  )

  target_link_libraries(pendule_pi
    pigpio::pigpio
    pthread
  )

  if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_definitions(pendule_pi PUBLIC PENDULE_PI_DEBUG_ENABLED)
  endif("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")

  # Build test programs
  add_subdirectory(test)
else(${pigpio_FOUND})
  message(WARNING "pigpio not found: source code will not be built.")
endif(${pigpio_FOUND})


#########################
# DOXYGEN DOCUMENTATION #
#########################

add_subdirectory(doc)
