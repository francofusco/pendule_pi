cmake_minimum_required(VERSION 3.15)
project(pendule_pi VERSION 1.0.0)

# Define some colors for output formatting
string(ASCII 27 Esc)
set(ColourReset "${Esc}[m")
set(Yellow      "${Esc}[33m")
set(Blue      "${Esc}[34m")


###################
# CONFIGURE BUILD #
###################
set(CMAKE_CXX_FLAGS_DEBUG_INIT "-Wall -Wpedantic -Wextra -Wshadow -Wconversion -Wcast-align -Wunnused -Wshadow -Wold-style-cast -Wpointer-arith -Wcast-qual -Wmissing-prototypes -Wno-missing-braces -fexceptions")


#######################
# LOCATE DEPENDENCIES #
#######################
find_package(pigpio QUIET)
find_package(iir_filters QUIET)

function(warn_if_not_found pkg)
  if(${${pkg}_FOUND})
    message(STATUS "${pkg}: found")
  else()
    message(STATUS "${pkg}: ${Yellow}not found${ColourReset}")
  endif()
endfunction()

if(${pigpio_FOUND} AND ${iir_filters_FOUND})
  set(ALL_DEPENDENCIES_FOUND TRUE)
else()
  message(STATUS "${Yellow}Some dependecies were not found (see below for details). Source code will not be built${ColourReset}")
  warn_if_not_found(pigpio)
  warn_if_not_found(iir_filters)
  set(ALL_DEPENDENCIES_FOUND FALSE)
endif()


#####################
# BUILD SOURCE CODE #
#####################


if(${ALL_DEPENDENCIES_FOUND})
  ################
  # MAIN LIBRARY #
  ################
  add_library(pendule_pi
    src/pendule_pi/joystick.cpp
    src/pendule_pi/pigpio.cpp
    src/pendule_pi/switch.cpp
    src/pendule_pi/motor.cpp
    src/pendule_pi/encoder.cpp
    src/pendule_pi/pendule.cpp
  )

  target_include_directories(pendule_pi PUBLIC
    include
  )

  target_link_libraries(pendule_pi
    pigpio::pigpio
    pthread
  )

  if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_options(pendule_pi PRIVATE -O0)
    target_compile_options(pendule_pi PUBLIC -Wall)
    target_compile_options(pendule_pi PUBLIC -Wpedantic)
    target_compile_options(pendule_pi PUBLIC -Wextra)
    target_compile_options(pendule_pi PUBLIC -Wshadow)
    target_compile_options(pendule_pi PUBLIC -Wconversion)
    target_compile_options(pendule_pi PUBLIC -Wcast-align)
    target_compile_options(pendule_pi PUBLIC -Wunused)
    target_compile_options(pendule_pi PUBLIC -Wshadow)
    target_compile_options(pendule_pi PUBLIC -Wold-style-cast)
    target_compile_options(pendule_pi PUBLIC -Wpointer-arith)
    target_compile_options(pendule_pi PUBLIC -Wcast-qual)
    target_compile_options(pendule_pi PUBLIC -Wno-missing-braces)
    target_compile_options(pendule_pi PUBLIC -fexceptions)
    target_compile_definitions(pendule_pi PUBLIC PENDULE_PI_DEBUG_ENABLED)
  elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    target_compile_options(pendule_pi PRIVATE -O2)
    # When Eigen will be used :P
    # target_compile_definitions(pendule_pi PRIVATE -DEIGEN_NO_DEBUG)
  endif()

  target_compile_features(pendule_pi PUBLIC cxx_std_17)

  ############
  # BINARIES #
  ############

  # Reset all pins in high impedance mode
  add_executable(reset_gpio src/bin/reset_gpio.cpp)
  target_link_libraries(reset_gpio pendule_pi)

  # Allows to control the pendulum with a joystick
  add_executable(manual_control src/bin/manual_control.cpp)
  target_link_libraries(manual_control pendule_pi)

  # Can be used to find the pwm threshold required to win static friction.
  add_executable(calibrate_pwm src/bin/calibrate_pwm.cpp)
  target_link_libraries(calibrate_pwm pendule_pi)

  # Record angular motions (useful for identification)
  add_executable(log_rotation src/bin/log_rotation.cpp)
  target_link_libraries(log_rotation pendule_pi)

  # Record angular motions (useful for identification)
  add_executable(log_motion src/bin/log_motion.cpp)
  target_link_libraries(log_motion pendule_pi)

  # Record angular motions (useful for identification)
  add_executable(log_joystick src/bin/log_joystick.cpp)
  target_link_libraries(log_joystick pendule_pi)

  # Linearized control
  add_executable(linear_control src/bin/linear_control.cpp)
  target_link_libraries(linear_control pendule_pi iir_filters::iir_filters)

  # Swingup + linearized control
  add_executable(swingup src/bin/swingup.cpp)
  target_link_libraries(swingup pendule_pi iir_filters::iir_filters)

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

  message(STATUS "${Blue}TODO: properly install the required files${ColourReset}")
  # install(TARGETS pendule_pi DESTINATION lib)
  # install(FILES ${pendule_pi_HEADERS} DESTINATION include)

  ###########
  # TESTING #
  ###########
  add_subdirectory(test)
endif(${ALL_DEPENDENCIES_FOUND})


#########################
# DOXYGEN DOCUMENTATION #
#########################

add_subdirectory(doc)
