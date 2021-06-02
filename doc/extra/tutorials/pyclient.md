@page pyclient Writing a Python client

[TOC]


# Introduction

This tutorial shows how to establish a connection with the low-level interface using the class `PenduleCpp`. The C++ client application will move the pendulum back and forth repeatedly, until CTRL-C is pressed to interrupt execution.


# Complete Code

The code for this tutorial is available in `examples/simple_connection/simple_connection.py`.

@include{lineno} simple_connection.py


# Detailed Explanation

@dontinclude{lineno} simple_connection.py

The program starts by importing the relevant modules:

@skip import
@until time

In particular, the first one provides the class `PendulePy` that allows one to connect to the low-level interface via sockets. An instance of this class is created in the "main":

@skipline PendulePy

Upon construction, the object will connect to the low level interface using default addresses (see @ref pendule_pi.PendulePy.__init__ "PendulePy's constructor"). Note that we pass a single integer parameter to the constructor. This represents the maximum amount of time to wait for the low-level interface to show up. If no messages are received within this amount of time, an exception will be thrown. You can pass a negative value (the default value is `-1`) in which case the `PendulePy` constructor will wait indefinitely for the low-level interface to show up.

We proceed by declaring two variables:

@skip pwm
@until switch_pos

The first one represents the intensity of the PWM command that we will send to the low-level interface (only its sign will be changed), while the second one corresponds to the distance from the center at which we want to invert the direction of the base.

We then enter a try-catch statement (needed to handle CTRL-C keyboard presses) and then the main loop, which is a `while` block that keeps running indefinitely. The code inside the loop is quite simple. We begin with the instruction

@skipline readState

which attempts to read a message sent from the low-level interface to update a couple of internal variables that store the current state of the pendulum. Since we pass `blocking=True` as parameter, the function will block execution until the message is received. If `blocking=False` was passed instead, the function would have firstly checked if a message was available and immediately returned `False` if no information was ready.

After the state has been updated, we can evaluate the "control":

@skip if
@until *=

The logic is really simple: if the pendulum is more than `switch_pos` meters to the right, and the current PWM command is positive (which pushes the base to the right) then we want to invert the command so that now the base moves leftwards. Similarly, if the base is more than `switch_pos` meters to the left and currently moving leftwards, then change the sign of `pwm` to invert the motion of the actuator.

Once the command has been evaluated, we just need to send it to the hardware interface:

@skipline sendCommand

After the while loop, we conclude the program by adding a catch block that intercepts `KeyboardInterrupt` exceptions (fired when you press CTRL-C). The block sends a null PWM to the low-level interface to stop the motion and sleeps for one second (just to make sure that the socket connection is not closed before the message is sent):

@skip except
@until sleep


# Running the code

@todo Explain how to run this code.
