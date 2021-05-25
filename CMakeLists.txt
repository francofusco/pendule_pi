### This file is an adaptation derived from
###   github.com/francofusco/template-cmake-project/blob/main/CMakeLists.txt
### Original Author: Franco FUSCO
### Source: github.com/francofusco/template-cmake-project
cmake_minimum_required(VERSION 3.19.4)
project(pendule_pi VERSION 1.0.0)

# Allow using colors in messages. Use with parsimony ;)
include(cmake/colors.cmake)

#######################
# LOCATE DEPENDENCIES #
#######################

# locate dependencies with a custom function
function(find_package_custom pkg)
  find_package(${pkg} QUIET)
  if(${${pkg}_FOUND})
    message(STATUS "${pkg}: ${Yellow}found${ColourReset}")
  else()
    set(ALL_DEPENDENCIES_FOUND FALSE)
    message(STATUS "${pkg}: ${Yellow}not found${ColourReset}")
  endif()
endfunction()

# Scan all dependecies
set(ALL_DEPENDENCIES_FOUND TRUE)
message(STATUS "Looking for build dependencies")
find_package_custom(pigpio)
find_package_custom(digital_filters)

# warn the user if code will not be built
if(NOT ${ALL_DEPENDENCIES_FOUND})
  message(STATUS "${Yellow}Some dependecies were not found. Source code will not be built${ColourReset}")
endif()




if(${ALL_DEPENDENCIES_FOUND})

#############
# LIBRARIES #
#############

# Main library
add_library(${PROJECT_NAME}
  src/pendule_pi/joystick.cpp
  src/pendule_pi/pigpio.cpp
  src/pendule_pi/switch.cpp
  src/pendule_pi/motor.cpp
  src/pendule_pi/encoder.cpp
  src/pendule_pi/pendule.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC pigpio::pigpio
  PUBLIC pthread
)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_compile_options(${PROJECT_NAME} PRIVATE -O0)
  target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
  target_compile_definitions(${PROJECT_NAME} PRIVATE PENDULE_PI_DEBUG_ENABLED)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  target_compile_options(${PROJECT_NAME} PRIVATE -O2)
  # If and when Eigen will be used :P
  # target_compile_definitions(${PROJECT_NAME} PRIVATE -DEIGEN_NO_DEBUG)
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)


############
# BINARIES #
############

# Reset all pins in high impedance mode
add_executable(reset_gpio src/bin/reset_gpio.cpp)
target_link_libraries(reset_gpio ${PROJECT_NAME})

# Allows to control the pendulum with a joystick
add_executable(manual_control src/bin/manual_control.cpp)
target_link_libraries(manual_control ${PROJECT_NAME})

# Record linear motions (useful for identification)
add_executable(log_motion src/bin/log_motion.cpp)
target_link_libraries(log_motion ${PROJECT_NAME})

# Record angular motions (useful for identification)
add_executable(log_rotation src/bin/log_rotation.cpp)
target_link_libraries(log_rotation ${PROJECT_NAME})

# Linearized control
add_executable(lqr_control src/bin/lqr_control.cpp)
target_link_libraries(lqr_control ${PROJECT_NAME} digital_filters::digital_filters)

# Swingup + linearized control
add_executable(swingup_and_lqr src/bin/swingup_and_lqr.cpp)
target_link_libraries(swingup_and_lqr ${PROJECT_NAME} digital_filters::digital_filters)


###########
# INSTALL #
###########

include(GNUInstallDirs)

# Install binaries
install(
  TARGETS
    reset_gpio
    manual_control
    log_motion
    log_rotation
    #lqr_control
    #swingup_and_lqr
  DESTINATION bin
)

# Installing the library
install(
  TARGETS ${PROJECT_NAME}
  DESTINATION lib
  EXPORT ${PROJECT_NAME}Targets
)

# Copy all headers from the "include" folder
install(
  DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/${PROJECT_NAME}
  DESTINATION include
)

# Install the "Targets" file as well.
install(
  EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  FILE ${PROJECT_NAME}Targets.cmake
  DESTINATION lib/cmake/${PROJECT_NAME}
)

# Helpers to create the "package config" files.
include(CMakePackageConfigHelpers)

# Create the "Config.cmake" file (to allow "find_package"-ing this project).
configure_package_config_file(${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "lib/cmake/example"
  NO_SET_AND_CHECK_MACRO
  NO_CHECK_REQUIRED_COMPONENTS_MACRO
)

# Generate the version file associated to the "Config.cmake" file.
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION "${${PROJECT_NAME}_VERSION_MAJOR}.${${PROJECT_NAME}_VERSION_MINOR}"
  COMPATIBILITY AnyNewerVersion
)

# Install the configuration file.
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  DESTINATION lib/cmake/${PROJECT_NAME}
)

# Make the project usable from the build directory as well.
export(
  EXPORT ${PROJECT_NAME}Targets
  NAMESPACE ${PROJECT_NAME}::
  FILE "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Targets.cmake"
)


# add the possibility tu run 'make uninstall' to remove files added via 'make install'
# NOTE: this will remove files only, and not their parent directories.
# WARNING: this will work only if you do not play around the installed targets manually,
# and if you do not change the install destination.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake
  IMMEDIATE @ONLY
)
add_custom_target(uninstall COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall.cmake")

endif(${ALL_DEPENDENCIES_FOUND})


#########################
# DOXYGEN DOCUMENTATION #
#########################

add_subdirectory(doc)
