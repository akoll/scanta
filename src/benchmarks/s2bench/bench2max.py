#!/bin/python

import sys
import numpy as np

print(np.max(np.array([float(line.rstrip()) for line in sys.stdin])))
