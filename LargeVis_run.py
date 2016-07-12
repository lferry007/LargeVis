import LargeVis
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('-fea', default = 1, type = int, help = 'whether to visualize high-dimensional feature vectors or networks')
parser.add_argument('-input', default = '', help = 'input file')
parser.add_argument('-output', default = '', help = 'output file')
parser.add_argument('-outdim', default = -1, type = int, help = 'output dimensionality')
parser.add_argument('-threads', default = -1, type = int, help = 'number of training threads')
parser.add_argument('-samples', default = -1, type = int, help = 'number of training mini-batches')
parser.add_argument('-prop', default = -1, type = int, help = 'number of propagations')
parser.add_argument('-alpha', default = -1, type = float, help = 'learning rate')
parser.add_argument('-trees', default = -1, type = int, help = 'number of rp-trees')
parser.add_argument('-neg', default = -1, type = int, help = 'number of negative samples')
parser.add_argument('-neigh', default = -1, type = int, help = 'number of neighbors in the NN-graph')
parser.add_argument('-gamma', default = -1, type = float, help = 'weight assigned to negative edges')
parser.add_argument('-perp', default = -1, type = float, help = 'perplexity for the NN-grapn')

args = parser.parse_args()

if args.fea == 1:
    LargeVis.loadfile(args.input)
else:
    LargeVis.loadgraph(args.input)

Y = LargeVis.run(args.outdim, args.threads, args.samples, args.prop, args.alpha, args.trees, args.neg, args.neigh, args.gamma, args.perp)

LargeVis.save(args.output)
