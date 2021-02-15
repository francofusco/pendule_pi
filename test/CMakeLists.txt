###################
# BUILD TEST CODE #
###################

# Check if we can read the state of a given GPIO
add_executable(test_pigpio_read
  test_pigpio_read.cpp
)

target_link_libraries(test_pigpio_read
  pendule_pi
)


# Check if the Switch class works
add_executable(test_switch
  test_switch.cpp
)

target_link_libraries(test_switch
  pendule_pi
)


# Check if the Motor class works
add_executable(test_motor
  test_motor.cpp
)

target_link_libraries(test_motor
  pendule_pi
)


# Check if the Motor and Switch classes work together
add_executable(test_switching_motor
  test_switching_motor.cpp
)

target_link_libraries(test_switching_motor
  pendule_pi
)


# Check if the Encoder class works
add_executable(test_encoder
  test_encoder.cpp
)

target_link_libraries(test_encoder
  pendule_pi
)


# Test if the Encoder class allows to return to the initial position
add_executable(test_encoder_homing
  test_encoder_homing.cpp
)

target_link_libraries(test_encoder_homing
  pendule_pi
)


# Test if the Encoder class allows to return to the initial position
add_executable(test_callback_exception
  test_callback_exception.cpp
)

target_link_libraries(test_callback_exception
  pendule_pi
)


# Test if the Encoder class allows to return to the initial position
add_executable(test_reset_pins
  test_reset_pins.cpp
)

target_link_libraries(test_reset_pins
  pendule_pi
)
