#!/bin/python

import sys
import numpy as np

print(np.average(np.array([float(line.rstrip()) for line in sys.stdin])))
