@page lowlevel Low-level interface

@warning The instructions inside this tutorial will likely be updated while moving to a more "stable" version of the low-level interface.

@todo describe the low-level interface.


# First time setup

@note Do not connect the transmission belt to the DC motor for this first setup. This will allow you to correctly configure the system without risking to damage the pendulum.

- SSH into the Raspberry (or directly open a terminal inside it).
- Move to the root folder of pendule_pi.
- Run `cp pendule_pi_config.yaml ~` to copy the configuration file `pendule_pi_config.yaml` into your home.
- Edit the file that you copied into your home. As an example, if you have gedit you might open it via `gedit ~/pendule_pi_config.yaml`. Another option would be VIM: `vim ~/pendule_pi_config.yaml`.
- Go to the build folder and run `sudo ./low_level_interface ~/pendule_pi_config.yaml`.
- Motor Phase: is the motor running in the correct direction? If not, invert the phases.
- Rotate the position encoder several times in the clockwise direction.
- Press the encoder switch.
- Rotate the encoder several times in the counter-clockwise direction.
- Press the motor switch.
- Check output and eventually adjust the setup (encoder phases and switches).
