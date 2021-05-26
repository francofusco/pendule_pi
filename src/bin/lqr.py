#!/usr/bin/env python3
from pendule_pi import Pendule
import math

def normalize(angle):
  while angle > math.pi:
    angle -= 2*math.pi
  while angle < -math.pi:
    angle += 2*math.pi
  return angle


if __name__ == '__main__':
  # Connect to the interface.
  pendulum = Pendule(wait=5)

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
