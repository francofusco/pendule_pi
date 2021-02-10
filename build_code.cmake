#################
# BUILD LIBRARY #
#################

add_library(pendule_pi
  src/pigpio.cpp
  src/switch.cpp
  src/motor.cpp
  src/encoder.cpp
)

target_include_directories(pendule_pi PUBLIC
  include
)

target_link_libraries(pendule_pi
  pigpio::pigpio
  pthread
  -Wl,--no-undefined
)


###################
# BUILD TEST CODE #
###################

add_executable(test_pigpio_exceptions
  test/test_pigpio_exceptions.cpp
)

target_link_libraries(test_pigpio_exceptions
  pendule_pi
)


add_executable(test_pigpio_read
  test/test_pigpio_read.cpp
)

target_link_libraries(test_pigpio_read
  pendule_pi
)


add_executable(test_switch
  test/test_switch.cpp
)

target_link_libraries(test_switch
  pendule_pi
)


add_executable(test_motor
  test/test_motor.cpp
)

target_link_libraries(test_motor
  pendule_pi
)


add_executable(test_switching_motor
  test/test_switching_motor.cpp
)

target_link_libraries(test_switching_motor
  pendule_pi
)
