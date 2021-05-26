#!/usr/bin/env python3
import zmq
import time
import sys

if __name__ == '__main__':
  # ZMQ connection
  context = zmq.Context()
  command_pub = context.socket(zmq.PUB)
  command_pub.connect("tcp://localhost:10002")
  state_sub = context.socket(zmq.SUB)
  state_sub.set(zmq.CONFLATE, 1)
  state_sub.connect("tcp://localhost:10001")
  state_sub.subscribe("")

  pwm = 25
  switch_pos = 0.2

  for i in range(10):
    msg = state_sub.recv_string()
    command_pub.send_string("0")
    time.sleep(0.1)

  try:
    while True:
      msg = state_sub.recv_string()
      position,angle,linvel,angvel = (float(v) for v in msg.split(" "))
      if (position > switch_pos and pwm > 0) or (position < -switch_pos and pwm < 0):
        pwm *= -1
      command_pub.send_string(str(pwm))
  except KeyboardInterrupt:
    print("Exiting")

  for i in range(10):
    msg = state_sub.recv_string()
    command_pub.send_string("0")
    time.sleep(0.1)
