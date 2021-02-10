cmake_minimum_required(VERSION 3.15)
project(pendule_pi VERSION 1.0.0)
add_definitions(-std=c++17)


#######################
# LOCATE DEPENDENCIES #
#######################

find_package(pigpio)


#####################
# BUILD SOURCE CODE #
#####################

if(${pigpio_FOUND})
  include(build_code.cmake)
else(${pigpio_FOUND})
  message(WARNING "pigpio not found: source code will not be built.")
endif(${pigpio_FOUND})


#########################
# DOXYGEN DOCUMENTATION #
#########################

add_subdirectory(doc)
