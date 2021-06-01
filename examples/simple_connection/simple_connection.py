#!/usr/bin/env python3
import pendule_pi
import time

if __name__ == '__main__':
  pendulum = pendule_pi.PendulePy(wait=5)

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
    time.sleep(1)
