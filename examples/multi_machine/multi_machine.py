#!/usr/bin/env python3
from pendule_pi import PendulePyClient
import time
import sys

if __name__ == '__main__':
  if len(sys.argv) != 2:
    print("Usage:", sys.argv[0], "host-name")
    sys.exit(0)
  pendulum = PendulePyClient(wait=5, host=sys.argv[1])

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
