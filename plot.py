#!/usr/bin/env python3

import argparse

import numpy as np
from matplotlib import rcParams, pyplot as plt

rcParams["svg.fonttype"] = "none"

parser = argparse.ArgumentParser()
parser.add_argument("--input", "-i", default="", help="input file", required=True)
parser.add_argument("--output", "-o", default="", help="output file", required=True)
parser.add_argument("--clusters", "-c", default="", help="clusters file")
parser.add_argument("--labels", "-l", default="", help="labels to annotate file")
parser.add_argument("--range", "-r", type=float, help="axis range")
parser.add_argument("--no-axis", "-n", help="hide axis", action="store_true")
parser.add_argument("--legend", "-s", help="show legend", action="store_true")
args = parser.parse_args()

clusters = {}
if args.clusters != "":
    with open(args.clusters) as f:
        for line in f:
            node, cluster = line.strip().split()
            clusters[node] = cluster

lables = []
if args.labels != "":
    with open(args.labels) as f:
        lables = list(map(lambda line: line.strip(), f))

positions_by_cluster = {}
positions = {}
with open(args.input) as f:
    _ = f.readline()  # ignore first line
    for line in f:
        vec = line.strip().split(" ")
        node = vec[0]
        pos = (float(vec[-2]), float(vec[-1]))
        positions[node] = pos
        positions_by_cluster.setdefault(clusters.get(node, ""), []).append(pos)

colors = plt.cm.tab10(np.linspace(0, 1, len(positions_by_cluster)))

for color, cluster in zip(colors, sorted(positions_by_cluster.keys())):
    x = [t[0] for t in positions_by_cluster[cluster]]
    y = [t[1] for t in positions_by_cluster[cluster]]
    plt.plot(x, y, ".", color=color, markersize=1, label=cluster)


for node in lables:
    x, y = positions[node]
    plt.annotate(node, xy=(x, y), xytext=(x - 3, y - 3), arrowprops=dict(arrowstyle="-"), fontsize="xx-small")

if args.range:
    axis_limit = abs(float(args.range))
    plt.xlim(-axis_limit, axis_limit)
    plt.ylim(-axis_limit, axis_limit)

if args.no_axis:
    plt.axis("off")

if args.legend:
    plt.legend()

plt.gca().set_aspect("equal", adjustable="box")
plt.savefig(args.output, dpi=500)
