@page cppclient Writing a C++ client

[TOC]


# Introduction

This tutorial shows how to establish a connection with the low-level interface using the class `PenduleCpp`. The C++ client application will move the pendulum back and forth repeatedly, until CTRL-C is pressed to interrupt execution.


# Complete Code

The code for this tutorial is available in `examples/simple_connection/simple_connection.cpp`.

@include simple_connection.cpp


# Detailed Explanation

@dontinclude simple_connection.cpp

The program starts by including the relevant headers:

@until thread

The first one provides the class `PenduleCpp`, while the other two allow to use `std::signal` and `std::this_thread_::sleep_for` (which will be used to gracefully shutdown the program).

Next come some lines that are used to handle the signal interruption:

@skipline ok
@until }

(Yes, I know, I know. Globals. @emoji :roll_eyes:) First we define a global variable `ok` whose initial value is `true` and then the function `sigintHandler` which simply changes `ok` into `false`. The reason for this is simple: in our code we will keep executing some actions as long as `ok` is `true`. To automatically execute `sigintHandler`, we register it as handler for `SIGINT` as the first instruction of our `main`:

@skipline signal

We then create an instance of `PenduleCpp`:

@skipline PenduleCpp

Upon construction, the object will connect to the low level interface using default addresses defined by the parameters `pendule_pi::PenduleCpp::DEFAULT_HOST`, `pendule_pi::PenduleCpp::DEFAULT_STATE_PORT` and `pendule_pi::PenduleCpp::DEFAULT_COMMAND_PORT`.
