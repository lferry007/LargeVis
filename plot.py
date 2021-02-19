#!/usr/bin/env python3

import argparse

import numpy
import matplotlib.pyplot as plt

parser = argparse.ArgumentParser()

parser.add_argument('--input', '-i', default='', help='input file', required=True)
parser.add_argument('--label', '-l', default='', help='label file')
parser.add_argument('--output', '-o', default='', help='output file', required=True)
parser.add_argument('--range', '-r', default='', help='axis range')

args = parser.parse_args()

labels = {}
if args.label != '':
    with open(args.label) as f:
        for line in f:
            node_id, label = line.strip().split()
            labels[node_id] = label

all_data = {}
with open(args.input) as f:
    _ = f.readline()  # ignore first line
    for line in f:
        vec = line.strip().split(' ')
        all_data.setdefault(labels.get(vec[0], 0), []).append((float(vec[-2]), float(vec[-1])))

colors = plt.cm.rainbow(numpy.linspace(0, 1, len(all_data)))

for color, ll in zip(colors, sorted(all_data.keys())):
    x = [t[0] for t in all_data[ll]]
    y = [t[1] for t in all_data[ll]]
    plt.plot(x, y, '.', color=color, markersize=1)
if args.range != '':
    l = abs(float(args.range))
    plt.xlim(-l, l)
    plt.ylim(-l, l)
plt.savefig(args.output, dpi=500)
