cmake_minimum_required(VERSION 3.15)
project(pendule_pi VERSION 1.0.0)
add_definitions(-std=c++17)

#######################
# LOCATE DEPENDENCIES #
#######################

find_package(pigpio REQUIRED)


#####################
# BUILD SOURCE CODE #
#####################

add_library(pendule_pi
  src/encoder.cpp
)

target_include_directories(pendule_pi PUBLIC
  include
  ${pigpio_INCLUDE_DIRS}
)

target_link_libraries(pendule_pi
  ${pigpio_LIBRARIES}
)


#########################
# DOXYGEN DOCUMENTATION #
#########################

add_subdirectory(doc)
