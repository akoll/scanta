#!/bin/python

import sys
import numpy as np

# print(np.average(np.array([float(line.rstrip()) for line in sys.stdin])))

repetitions = int(sys.argv[1]) if len(sys.argv) > 1 else 1

values = np.array([float(line.rstrip()) for line in sys.stdin])
values = values.reshape(repetitions, values.shape[0] // repetitions)

averages = np.average(values, axis=0)

if repetitions == 1:
  print(averages)
else:
  for avg in averages:
    print(avg)
