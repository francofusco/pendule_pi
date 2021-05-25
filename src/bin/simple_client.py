#!/usr/bin/env python3
import zmq
import time
import sys

if __name__ == '__main__':
  period = 1.0 if len(sys.argv) < 2 else float(sys.argv[1])
  print("Main loop period:", period)

  context = zmq.Context()

  pub = context.socket(zmq.PUB)
  pub.connect("tcp://localhost:10002")
  pub_count = 0

  sub = context.socket(zmq.SUB)
  sub.set(zmq.CONFLATE, 1)
  sub.connect("tcp://localhost:10001")
  sub.subscribe("")

  while True:
    print("----------")
    pub_count += 1
    msg_str = f"Sent from py-client {pub_count}"
    pub.send_string(msg_str)
    print("Sent message:", msg_str)

    try:
      msg = sub.recv_string(flags=zmq.NOBLOCK)
      print("Received message:", msg)
    except zmq.Again:
        print("No message received")
    time.sleep(period)
