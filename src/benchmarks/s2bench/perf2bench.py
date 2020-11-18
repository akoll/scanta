#!/bin/python

import sys
import re


only_loads = sys.argv[1] == 'loads' if len(sys.argv) > 1 else False

loads = None
misses = None

for line in sys.stdin:
  match = re.match('^(\\d+),.*L1-dcache-loads.*$', line)
  if match:
    loads = float(match.group(1))
  match = re.match('^(\\d+),.*L1-dcache-load-misses.*$', line)
  if match:
    misses = float(match.group(1))

if only_loads:
  if loads:
    print(loads)
  else:
    exit(1)
else:
  if loads and misses:
    print((misses / loads) * 100)
  else:
    exit(1)
