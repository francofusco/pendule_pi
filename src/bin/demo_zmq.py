#!/usr/bin/env python3
from pendule_pi import PendulePy


if __name__ == '__main__':
  # Connect to the interface.
  pendulum = PendulePy(wait=5)

  pwm = 25
  switch_pos = 0.2

  try:
    while True:
      pendulum.readState(blocking=True)
      if (pendulum.position > switch_pos and pwm > 0) or (pendulum.position < -switch_pos and pwm < 0):
        pwm *= -1
      pendulum.sendCommand(pwm)
  except KeyboardInterrupt:
    pendulum.sendCommand(0)
