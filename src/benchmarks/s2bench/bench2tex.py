#!/bin/python

import sys

print(r'\addplot[thin,%s] coordinates {' % sys.argv[1])

xfunc = sys.argv[2] if len(sys.argv) > 2 else 'x'
xfunc = 'lambda x: {}'.format(xfunc)
xfunc = eval(xfunc)

index = 0
for line in sys.stdin:
  print('  ({}, {})'.format(xfunc(index), line.rstrip()))
  index += 1

print('};')
