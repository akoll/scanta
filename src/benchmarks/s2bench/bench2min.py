#!/bin/python

import sys
import numpy as np

print(np.min(np.array([float(line.rstrip()) for line in sys.stdin])))
