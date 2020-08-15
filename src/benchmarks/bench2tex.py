#!/bin/python

import sys

print(r'\addplot[thin] coordinates {')

index = 0
for line in sys.stdin:
  if index >= 0:
    print('  ({}, {})'.format(index, line.rstrip()))
  index += 1

print('};')