@page build_pendulum Build the Pendulum

[TOC]

# Introduction

@todo Write the tutorial to build the physical pendulum!

After installing the parts and wiring, please verify the motor direction as well as the encoders increments.

# Components
| Name                |        Model        |                                                                                      Function |
|:--------------------|:-------------------:|---------------------------------------------------------------------------------------------:|
| 12V DC power source |         N/A         |                                         DC motor power source, i.e converts 230V AC to 12V DC |
| Raspberry Pi 4      |   Raspberry Pi 4    |                      Pilots the DC motor, manages interrupts and communicates with the laptop |
| Positional encoders | LD3806-600BM-G5-24C | One encoder is responsible for the measuring the θ of the pendulum, another for "x" of a cart |
| DC motor            |     MFA 970D12V     |                                                            move the cart through toothed belt |
| limit sensors       |    OMRON SS-5GL     |                Security measure interrupt to stop the motor if it reaches the end of the rail |
| DC motor driver     |    OMRON SS-5GL     |           |
# 3D printing
We printed all plastic parts with the 100% density with curra software.
# Assemble the parts

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
