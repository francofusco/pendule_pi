cmake_minimum_required(VERSION 3.19.4)
project(pendule_pi_examples VERSION 1.0.0)

find_package(pendule_pi REQUIRED)

function(add_example exe)
  add_executable(${exe} ${exe}/${exe}.cpp)
  target_link_libraries(${exe} PUBLIC pendule_pi::pendule_cpp)
endfunction()

add_example(simple_connection)
add_example(lqr)
add_example(multi_machine)
