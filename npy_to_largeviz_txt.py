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
import logging
import sys

import numpy as np


logger = logging.getLogger(__name__)


def npy_to_largeviz_txt(filename_input, filename_output=None):
    logger.info("npy_to_largeviz_txt() on %s", filename_input)
    data = np.load(filename_input)
    if not filename_output:
        filename_output = '%s.txt' % filename_input
    logger.info("Saving result to %s", filename_output)
    with open(filename_output, 'w') as fp:
        writer = csv.writer(fp, delimiter=' ')
        writer.writerow(data.shape)
        for vector in data:
            writer.writerow(vector.tolist())


def largeviz_txt_to_npy(filename_input, filename_output=None):
    logger.info("largeviz_txt_to_npy() on %s", filename_input)
    with open(filename_input, 'r') as fp:
        reader = csv.reader(fp, delimiter=' ')
        shape = map(int, reader.next())
        logger.info("shape=%s", shape)
        vectors = np.zeros(shape)
        for row_num, row in enumerate(reader):
            vectors[row_num, :] = np.array(row)
    if not filename_output:
        filename_output = '%s.npy' % filename_input
    logger.info("Saving result to %s", filename_output)
    np.save(filename_output, vectors)


def setup_logger():
    logger.setLevel(logging.INFO)
    logger.handlers = []
    formatter = logging.Formatter('%(asctime)s [%(levelname)s]: %(message)s',
                                  '%Y-%m-%d %H:%M:%S')
    stream_handler = logging.StreamHandler()
    stream_handler.setLevel(logging.INFO)
    stream_handler.setFormatter(formatter)
    logger.addHandler(stream_handler)


def main():
    setup_logger()
    filename_input = sys.argv[1]
    filename_output = len(sys.argv) > 2 and sys.argv[2]
    if filename_input.endswith('.txt'):
        largeviz_txt_to_npy(filename_input, filename_output)
    else:
        npy_to_largeviz_txt(filename_input, filename_output)


if __name__ == '__main__':
    main()
