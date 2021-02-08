cmake_minimum_required(VERSION 3.15)
project(pendule_pi VERSION 1.0.0)

# find_library(pigpio REQUIRED)

message(STATUS "Hello")

add_library(pendule_pi
  src/version.cpp
  src/encoder.cpp
  # src/motor.cpp
)

target_include_directories(pendule_pi PUBLIC
  include
  # ${pigpio_INCLUDE_DIRS}
)

# target_link_libraries(pendule_pi
#   ${pigpio_LIBRARIES}
# )



add_executable(joystick_control
  src/joystick_control_main.cpp
  # src/joystick_control.cpp
)

target_include_directories(joystick_control PUBLIC
  include
  # ${pigpio_INCLUDE_DIRS}
)

target_link_libraries(joystick_control
  pendule_pi
  # ${pigpio_LIBRARIES}
)
