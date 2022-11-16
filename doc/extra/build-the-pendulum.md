[//]: # (@page build_pendulum Build the Pendulum)

[TOC]

# Introduction

@todo Write the tutorial to build the physical pendulum!
Look at the joint video and the provided link with the CAD drawing of assembly to guide you in the process of printing the elements and their assembly.

After installing the parts and wiring, please verify the motor direction as well as the encoders increments.

# Components 
| Name                |        Model        |                                                                                           Function |
|:--------------------|:-------------------:|---------------------------------------------------------------------------------------------------:|
| 12V DC power source |         N/A         |                                              DC motor power source, i.e converts 230V AC to 12V DC |
| Raspberry Pi 4      |   Raspberry Pi 4    |                           Pilots the DC motor, manages interrupts and communicates with the laptop |
| Positional encoders | LD3806-600BM-G5-24C |      One encoder is responsible for the measuring the θ of the pendulum, another for "x" of a cart |
| DC motor            |     MFA 970D12V     |                                                                 move the cart through toothed belt |
| limit sensors       |    OMRON SS-5GL     |                     Security measure interrupt to stop the motor if it reaches the end of the rail |
| DC motor driver     |    OMRON SS-5GL     | receive PWM signal from raspberry and modulate the DC voltage accordingly to power-up the DC motor |
| pendulum rod        |    OMRON SS-5GL     | receive PWM signal from raspberry and modulate the DC voltage accordingly to power-up the DC motor |

# 3D printing
We printed all plastic parts with the 100% density with curra software.
parts to be printed in PLA material:
* motor-support
* encoder-support
* 2 x belt-cap
* pendulum details: int-part#1,int-part#2, pendulum-head#1,pendulum-head#2
in ABS
* support-pendule


# Assemble the parts
open .stp(location: ./doc/extra/cartPole.stp) file in CAD software
or [online](https://cad.onshape.com/documents/cb7bd0e195c60437cf97c9d5/w/e6da1cebe26dd182a7bf81c1/e/3d69146d5de92928caf63491)(after creating onshape account)

steps:
* fix the metalic part of the cart on the rail
* fix the metalic rail on the supports of motor and encoder
* fix the motor and x-positional encoder on the supports
* fix the whole assembly tightly on the table (for example: with the help of 2-sided tape)
* fix the pendulum support on the metallic part
* install PLA head parts on the pendulum rod
# Wiring
The correct wiring is crucial for the proper functioning of the software. The wires can be connected/soldered directoly on the GPIO header or on an extension board.

| GPIO                |        function        |  
|--------------------|-------------------|
|24|PWM of the driver|
|17|limit sensor|
|18|limit sensor|
|19|encoder pendulum|
|20|encoder cart|
|21|encoder cart|
|26|encoder pendulum|
