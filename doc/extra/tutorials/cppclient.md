@page cppclient Writing a C++ client

[TOC]


# Introduction

This tutorial shows how to establish a connection with the low-level interface using the class `PenduleCppClient`. The C++ client application will move the pendulum back and forth repeatedly, until CTRL-C is pressed to interrupt execution.


# Complete Code

The code for this tutorial is available in `examples/simple_connection/simple_connection.cpp`.

@include{lineno} simple_connection.cpp


# Detailed Explanation

@dontinclude{lineno} simple_connection.cpp

The program starts by including the relevant headers:

@until thread

The first one provides the class `PenduleCppClient`, while the other two allow to use `std::signal` and `std::this_thread_::sleep_for` (which will be used to gracefully shutdown the program).

Next come some lines that are used to handle the signal interruption:

@skipline ok
@until }

(Yes, I know, I know. Globals. @emoji :roll_eyes:) First we define a global variable `ok` whose initial value is `true` and then the function `sigintHandler` which simply changes `ok` into `false`. The reason for this is simple: in our code we will keep executing some actions as long as `ok` is `true`. To automatically execute `sigintHandler`, we register it as handler for `SIGINT` as the first instruction of our `main`:

@skipline signal

We then create an instance of `PenduleCppClient`:

@skipline PenduleCppClient

Upon construction, the object will connect to the low level interface using default addresses defined by the parameters `pendule_pi::PenduleCppClient::DEFAULT_HOST`, `pendule_pi::PenduleCppClient::DEFAULT_STATE_PORT` and `pendule_pi::PenduleCppClient::DEFAULT_COMMAND_PORT`. Note that we pass a single integer parameter to the constructor. This represents the maximum amount of time to wait for the low-level interface to show up. If no messages are received within this amount of time, an exception will be thrown. You can pass a negative value (the default value is `-1`) in which case the `PenduleCppClient` constructor will wait indefinitely for the low-level interface to show up.

We proceed by declaring two variables:

@skip pwm
@until switch_pos

The first one represents the intensity of the PWM command that we will send to the low-level interface (only its sign will be changed), while the second one corresponds to the distance from the center at which we want to invert the direction of the base.

We then enter the main loop, which is a `while` block that keeps running as long as `ok` is `ŧrue`, *i.e.*, until CTRL-C is pressed on the keyboard:

@skipline while

The code inside the loop is quite simple. We begin with the instruction

@skipline readState

which attempts to read a message sent from the low-level interface to update a couple of internal variables that store the current state of the pendulum. Since we pass `pendulum.BLOCKING` as parameter, the function will block execution until the message is received. If `pendulum.NON_BLOCKING` was passed instead, the function would have firstly checked if a message was available and immediately returned `false` if no information was ready.

After the state has been updated, we can evaluate the "control":

@skip if
@until *=

The logic is really simple: if the pendulum is more than `switch_pos` meters to the right, and the current PWM command is positive (which pushes the base to the right) then we want to invert the command so that now the base moves leftwards. Similarly, if the base is more than `switch_pos` meters to the left and currently moving leftwards, then change the sign of `pwm` to invert the motion of the actuator.

Once the command has been evaluated, we just need to send it to the hardware interface:

@skipline sendCommand

After closing the while loop, we conclude the program by sending a null PWM to the low-level interface to stop the motion and sleep for one second (just to make sure that the socket connection is not closed before the message is sent):

@skip sendCommand
@until sleep_for


# Running the code

@todo Explain how to build and run this code.
