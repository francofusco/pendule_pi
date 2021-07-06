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
    message(STATUS "${pkg}: ${Green}found${ColourReset}")
  else()
    set(ALL_DEPENDENCIES_FOUND FALSE PARENT_SCOPE)
    message(STATUS "${pkg}: ${Yellow}not found${ColourReset}")
  endif()
endfunction()

# Scan all dependecies
set(ALL_DEPENDENCIES_FOUND TRUE)
message(STATUS "Looking for build dependencies")
find_package_custom(pigpio)
find_package_custom(digital_filters)
find_package_custom(yaml-cpp)

# warn the user if code will not be built
if(NOT ${ALL_DEPENDENCIES_FOUND})
  message(STATUS "${Yellow}Some dependecies were not found. Only C++ bridges will be built!${ColourReset}")
endif()

##############
# C++ client #
##############

add_library(pendule_cpp_client src/pendule_pi/pendule_cpp_client.cpp)

target_include_directories(pendule_cpp_client
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(pendule_cpp_client zmq zmqpp)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_compile_options(pendule_cpp_client PRIVATE -O0)
  target_compile_options(pendule_cpp_client PRIVATE -Wall)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  target_compile_options(pendule_cpp_client PRIVATE -O2)
endif()

target_compile_features(pendule_cpp_client PUBLIC cxx_std_17)

##############
# C++ server #
##############

add_library(pendule_cpp_server src/pendule_pi/pendule_cpp_server.cpp)

target_include_directories(pendule_cpp_server
  PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)

target_link_libraries(pendule_cpp_server zmq zmqpp)

if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  target_compile_options(pendule_cpp_server PRIVATE -O0)
  target_compile_options(pendule_cpp_server PRIVATE -Wall)
elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
  target_compile_options(pendule_cpp_server PRIVATE -O2)
endif()

target_compile_features(pendule_cpp_server PUBLIC cxx_std_17)

##############
# pendule_pi #
##############

if(${ALL_DEPENDENCIES_FOUND})

  add_library(${PROJECT_NAME}
    src/pendule_pi/config.cpp
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
    PUBLIC zmq
    PUBLIC zmqpp
    PUBLIC yaml-cpp
    PUBLIC digital_filters::digital_filters
    PUBLIC pendule_cpp_server
  )

  if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
    target_compile_options(${PROJECT_NAME} PRIVATE -O0)
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall)
    target_compile_definitions(${PROJECT_NAME} PUBLIC PENDULE_PI_DEBUG_ENABLED)
  elseif("${CMAKE_BUILD_TYPE}" STREQUAL "Release")
    target_compile_options(${PROJECT_NAME} PRIVATE -O2)
    # If and when Eigen will be used :P
    # target_compile_definitions(${PROJECT_NAME} PRIVATE -DEIGEN_NO_DEBUG)
  endif()

  target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

endif(${ALL_DEPENDENCIES_FOUND})


############
# BINARIES #
############

if(${ALL_DEPENDENCIES_FOUND})

  # Reset all pins in high impedance mode
  add_executable(reset_gpio src/bin/reset_gpio.cpp)
  target_link_libraries(reset_gpio ${PROJECT_NAME})

  # Executable that can help to setup the pendulum.
  add_executable(pendule_pi_assistant src/bin/pendule_pi_assistant.cpp)
  target_link_libraries(pendule_pi_assistant ${PROJECT_NAME})

  # Swingup + linearized control
  add_executable(demo src/bin/demo.cpp)
  target_link_libraries(demo ${PROJECT_NAME} pendule_cpp_client)

  add_executable(low_level_interface src/bin/low_level_interface.cpp)
  target_link_libraries(low_level_interface ${PROJECT_NAME})

endif(${ALL_DEPENDENCIES_FOUND})


if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/piksel")
  message(STATUS "piksel: ${Green}found${ColourReset} (building simulator as well).")
  add_subdirectory(piksel)

  add_executable(simulator src/bin/simulator.cpp)
  target_include_directories(simulator PUBLIC piksel/src)
  target_link_libraries(simulator
    PUBLIC piksel
    PUBLIC pendule_cpp_server
    PUBLIC yaml-cpp
  )
else()
  message(STATUS "piksel: ${Yellow}not found${ColourReset} (the simulator will not be built).")
endif()


###########
# INSTALL #
###########

include(GNUInstallDirs)

# Install binaries
if(${ALL_DEPENDENCIES_FOUND})
  install(
    TARGETS
      reset_gpio
      pendule_pi_assistant
      demo
    DESTINATION bin
  )
endif(${ALL_DEPENDENCIES_FOUND})

# Install the libraries
set(LIBS_TO_INSTALL pendule_cpp_client pendule_cpp_server)
if(${ALL_DEPENDENCIES_FOUND})
  list(APPEND LIBS_TO_INSTALL ${PROJECT_NAME})
endif(${ALL_DEPENDENCIES_FOUND})

install(
  TARGETS ${LIBS_TO_INSTALL}
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


# Add the possibility tu run 'make uninstall_pendule_pi' to remove files added
# via 'make install'.
# NOTE: this will remove files only, and not their parent directories.
# WARNING: this will work only if you do not play around the installed
# targets manually, and if you do not change the install destination.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/uninstall_${PROJECT_NAME}.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/uninstall_${PROJECT_NAME}.cmake
  IMMEDIATE @ONLY
)
add_custom_target(uninstall_${PROJECT_NAME} COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_BINARY_DIR}/uninstall_${PROJECT_NAME}.cmake")



#########################
# DOXYGEN DOCUMENTATION #
#########################

add_subdirectory(doc)
