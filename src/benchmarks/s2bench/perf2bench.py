#!/bin/python

import sys
import re

loads = None
misses = None

for line in sys.stdin:
  match = re.match('^(\\d+),.*L1-dcache-loads.*$', line)
  if match:
    loads = float(match.group(1))
  match = re.match('^(\\d+),.*L1-dcache-load-misses.*$', line)
  if match:
    misses = float(match.group(1))

if loads and misses:
  print((misses / loads) * 100)
else:
  exit(1)
