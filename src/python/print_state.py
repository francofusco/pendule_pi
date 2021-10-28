#!/usr/bin/env python3
from pendule_pi import PendulePyClient
import time
import sys

if __name__ == '__main__':
  host = sys.argv[1] if len(sys.argv) > 1 else "localhost"
  pendulum = PendulePyClient(wait=5, host=host)

  print(f" POSITION       ANGLE      LIN.VEL.     ANG.VEL.")
  # print(f"12345678901  12345678901  12345678901  12345678901")

  try:
    while True:
      pendulum.readState(blocking=True)
      print(f"{pendulum.position:0=11.5f}  {pendulum.angle:0=11.5f}  {pendulum.linvel:0=11.5f}  {pendulum.angvel:0=11.5f}", end='\r', flush=True)
  except KeyboardInterrupt:
    pass
