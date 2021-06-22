@page multimachine Multi-machine setup

[TOC]

# Introduction

This tutorial discusses how you can setup an external PC so that it can discuss with the low-level interface and control the pendulum.

The steps are rather simple but can be confusing. Make sure to read them carefully: do not just execute the commands in the terminal without having understood what you are doing! @emoji :wink:

It is assumed here that you were able to compile and install pendule_pi on the Raspberry Pi (see @ref installation) and that other examples (such as @ref cppclient or @ref pyclient) were working as expected.

# Setup

Before showing the code for the tutorial, make sure that your machine is properly configured. The main requirement is that you are able to ping the Raspberry from your machine, either via an IP address or by a name.

After having checked that you can ping the board, ensure that you can compile and run the code on your local machine. For the rest of the tutorial *unless specified otherwise* commands are to be run on a "regular" terminal on your PC, *i.e.*, do not use ssh and do not work on the Raspberry.

The required steps are just a subset of the installation instructions. In particular, you should should firstly ensure to have the following libraries installed (once again: on your PC, not on the Raspberry!):

- @ref install_cmake
- @ref install_python3
- @ref install_zeromq

Once these dependencies are installed, you can clone pendule_pi on your machine and build it. Since we know that you are a good person and have already read the build instructions thoroughly, you can jump to the @ref install_pendule_pi_tldr section and just run the instructions listed there. Note that, unless your machine is another Raspberry and you have installed also pigpio and digital_filters, your PC will not build the `pendule_pi` library. Instead, it will build the target `pendule_cpp_client`, which is the library that contains the code for the `PenduleCppClient` class. This is absolutely fine since on your PC you need nothing more than this to talk with the low-level interface @emoji :relaxed:

You can check your setup by moving inside the `examples` directory and by compiling them on your PC (see the @ref tutorials "Tutorials page"). If no error is given and the code compiles fine, you can move to one of the next sections, which show how to connect to the low-level interface using either Python or C++.


# The Code (Python)

In the following, you will see a (slightly) modified version of the code shown in the tutorial @ref pyclient. The only difference is in the address used to connect to the low-level interface.


## Complete Code

The code for this tutorial is available in `examples/multi_machine/multi_machine.py`.

@include{lineno} multi_machine.py


## Detailed Explanation

@dontinclude{lineno} multi_machine.py

Most of the code is already explained in @ref pyclient. We will only discuss the part related to the host setup.

After importing some modules, the script starts with:

@skip sys.argv
@until sys.exit

This executable assumes that one runs it as:
```
python3 multi_machine.py some_host
```
where `some_host` identifies the Raspberry that is running the low-level interface. This host is then passed to the @ref pendule_pi.PendulePyClient.__init__ "constructor of PendulePyClient":

@skipline PendulePyClient

States can now be read from the address `tcp://some_host:10001` and commands can be sent to `tcp://some_host:10002`. If `some_host` properly identifies the Raspberry, then the rest of the script will be able to move the base of the pendulum around!


## Running the Example

First make sure that the low-level interface is running (see @ref lowlevel).

@warning @emoji :warning: The low-level interface is the only component of this tutorial that has to be started in the Raspberry. You can open a terminal into your machine, ssh into the micro-computer and launch the interface from there. However, make sure not to launch anything else from the Raspberry @emoji :wink:

Once the calibration is completed and the pendulum is ready to accept commands, move (in your PC!) to the folder `examples/multi_machine`. Then, start the script via:
```
python3 multi_machine.py host_name
```
Remember to change `host_name` with your Raspberry's IP address or with the name associated to it.


# The Code (C++)

In the following, you will see a (slightly) modified version of the code shown in the tutorial @ref cppclient. The only difference is in the address used to connect to the low-level interface.


## Complete Code

The code for this tutorial is available in `examples/multi_machine/multi_machine.cpp`.

@include{lineno} multi_machine.cpp


## Detailed Explanation

@dontinclude{lineno} multi_machine.cpp

Most of the code is already explained in @ref cppclient. We will only discuss the part related to the host setup.

After including some headers, the program starts with:

@skip if
@until }

This executable assumes that one runs it as:
```
./multi_machine some_host
```
where `some_host` identifies the Raspberry that is running the low-level interface. This host is then passed to the @ref pendule_pi::PenduleCppClient::PenduleCppClient "constructor of PenduleCppClient":

@skip PenduleCppClient
@until )

States can now be read from the address `tcp://some_host:10001` and commands can be sent to `tcp://some_host:10002`. If `some_host` properly identifies the Raspberry, then the rest of the script will be able to move the base of the pendulum around!


## Running the Example

First make sure that the low-level interface is running (see @ref lowlevel).

@warning @emoji :warning: The low-level interface is the only component of this tutorial that has to be started in the Raspberry. You can open a terminal into your machine, ssh into the micro-computer and launch the interface from there. However, make sure not to launch anything else from the Raspberry @emoji :wink:

Once the calibration is completed and the pendulum is ready to accept commands, move (in your PC!) to the folder `examples/build`. Then, start the program via:
```
./multi_machine host_name
```
Remember to change `host_name` with your Raspberry's IP address or with the name associated to it.
