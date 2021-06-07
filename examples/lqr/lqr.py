#!/usr/bin/env python3
from pendule_pi import PendulePy
import math
import time
import sys

def normalize(angle):
  while angle > math.pi:
    angle -= 2*math.pi
  while angle < -math.pi:
    angle += 2*math.pi
  return angle


if __name__ == '__main__':
  if len(sys.argv) != 2:
    print("Usage:", sys.argv[0], "host-name")
    sys.exit(0)
  pendulum = pendule_pi.PendulePy(wait=5, host=sys.argv[1])

  MAX_ANGLE = 0.1
  kp = -127.45880662905581
  kpd = -822.638944546691
  kt = 2234.654627319883
  ktd = 437.1177135919267

  try:
    while True:
      pendulum.readState(blocking=True)
      et = normalize(pendulum.angle-math.pi)
      if abs(et) < MAX_ANGLE:
        pwm = - kp * pendulum.position - kt * et - kpd * pendulum.linvel - ktd * pendulum.angvel
      else:
        pwm = 0
      pendulum.sendCommand(pwm)
  except KeyboardInterrupt:
    pendulum.sendCommand(0)
    time.sleep(1)
