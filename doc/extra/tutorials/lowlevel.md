@page lowlevel Low-level interface

@warning The instructions inside this tutorial will likely be updated while moving to a more "stable" version of the low-level interface.


# First time setup

@note It is highly recommended that you disconnect the transmission belt from the DC motor for this first setup and that you detach the weight and rod from the pivot. This will allow you to correctly configure the system without risking to damage the pendulum.

## YAML configuration file

First of all, open a terminal in the Raspberry (or SSH into it) and move to the root folder of %pendule_pi. You will find a YAML file named `pendule_pi_config.yaml`. Make a copy of it outside the repository, preferably in your home (so that it will be faster to locate it). For the rest of the tutorial, we will assume that you copied the file into your home using `cp pendule_pi_config.yaml ~`. If you decided to store the file under a different location/name, make sure to change `~/pendule_pi_config.yaml` with the correct file path when it shows up in the sequel.

Now, move to the `build` folder and start the interface by running:
```
sudo ./low_level_interface ~/pendule_pi_config.yaml
```
If you receive errors, try checking the @ref troubleshooting page. Otherwise, you should see that the motor starts spinning.

@note The motor should spin in a way that would "push" the cart away, towards the position encoder. If this is not the case, please invert the polarity of the motor, *i.e.*, swap its two cables on the motor control board.

If the pendulum was fully assembled (which is not, because you followed our note and disconnected the transmission belt @emoji :wink:), you would now see the cart moving towards the center 



- Edit the file that you copied into your home. As an example, if you have gedit you might open it via `gedit ~/pendule_pi_config.yaml`. Another option would be VIM: `vim ~/pendule_pi_config.yaml`.
- Go to the build folder and run `sudo ./low_level_interface ~/pendule_pi_config.yaml`.
- Motor Phase: is the motor running in the correct direction? If not, invert the phases.
- Rotate the position encoder several times in the clockwise direction.
- Press the encoder switch.
- Rotate the encoder several times in the counter-clockwise direction.
- Press the motor switch.
- Check output and eventually adjust the setup (encoder phases and switches).
