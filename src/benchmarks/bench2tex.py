#!/bin/python

import sys

print(r'\addplot[thin,%s] coordinates {' % sys.argv[1])

multiplier = int(sys.argv[2]) if len(sys.argv) > 2 else 1

index = 0
for line in sys.stdin:
  if index >= 0:
    print('  ({}, {})'.format(index * multiplier, line.rstrip()))
  index += 1

print('};')
