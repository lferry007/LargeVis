#!/usr/bin/env python

"""Tool for converting a numpy array file `input.npy` to the type of file that
LargeViz expects as input, `input.npy.txt`.

TODO: `.npz` support

References:

- https://docs.scipy.org/doc/numpy/reference/generated/numpy.savez.html
- https://docs.scipy.org/doc/numpy/reference/generated/numpy.save.html
- https://docs.scipy.org/doc/numpy/reference/generated/numpy.load.html
- https://docs.scipy.org/doc/numpy/neps/npy-format.html

"""

import csv
import sys

import numpy as np


def npy_to_largeviz_input(filename_input, filename_output=None):
    data = np.load(filename_input)
    if not filename_output:
        filename_output = '%s.txt' % filename_input
    with open(filename_output, 'w') as fp:
        writer = csv.writer(fp, delimiter=' ')
        writer.writerow(data.shape)
        for vector in data:
            writer.writerow(vector.tolist())


def main():
    filename_input = sys.argv[1]
    filename_output = len(sys.argv) > 2 and sys.argv[2]
    npy_to_largeviz_input(filename_input, filename_output)


if __name__ == '__main__':
    main()
