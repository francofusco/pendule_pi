cmake_minimum_required(VERSION 3.15)
project(pendule_pi VERSION 1.0.0)


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

  target_compile_features(pendule_pi PUBLIC cxx_std_17)

  ###########
  # INSTALL #
  ###########

  # Allow other projects to find and install pendule_pi
  export(TARGETS pendule_pi
    NAMESPACE pendule_pi::
    FILE ${PROJECT_BINARY_DIR}/pendule_piDepends.cmake
  )
  configure_file(pendule_piConfig.cmake.in ${PROJECT_BINARY_DIR}/pendule_piConfig.cmake @ONLY)
  export(PACKAGE pendule_pi)

  message(WARNING "${Yellow}TODO: properly install the required files${ColourReset}")
  # install(TARGETS pendule_pi DESTINATION lib)
  # install(FILES ${pendule_pi_HEADERS} DESTINATION include)

  ###########
  # TESTING #
  ###########
  add_subdirectory(test)
else(${pigpio_FOUND})
  message(WARNING "pigpio not found: source code will not be built.")
endif(${pigpio_FOUND})


#########################
# DOXYGEN DOCUMENTATION #
#########################

add_subdirectory(doc)
