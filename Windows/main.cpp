#include "LargeVis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char infile[1000], outfile[1000];
long long if_embed = 1, out_dim = -1, n_samples = -1, n_threads = -1, n_negative = -1, n_neighbors = -1, n_trees = -1, n_propagation = -1;
real alpha = -1, n_gamma = -1, perplexity = -1;

int ArgPos(char *str, int argc, char **argv) {
	int a;
	for (a = 1; a < argc; a++) if (!strcmp(str, argv[a])) {
		if (a == argc - 1) {
			printf("Argument missing for %s\n", str);
			exit(1);
		}
		return a;
	}
	return -1;
}

int main(int argc, char **argv)
{
	long long i;
    if (argc < 3)
    {
        printf("-fea: specify whether the input file is high-dimensional feature vectors (1) or networks (0). Default is 1.\n");
        printf("-input: Input file of feature vectors or networks\n");
        printf("-output: Output file of low-dimensional representations.\n");
        printf("-threads: Number of threads. Default is 8.\n");
        printf("-outdim: The lower dimensionality LargesVis learns for visualization (usually 2 or 3). Default is 2.\n");
        printf("-samples: Number of edge samples for graph layout (in millions). Default is set to data size / 100 (million).\n");
        printf("-prop: Number of times for neighbor propagations in the state of K-NNG construction, usually less than 3. Default is 3.\n");
        printf("-alpha: Initial learning rate. Default is 1.0.\n");
        printf("-trees: Number of random-projection trees used for constructing K-NNG. 50 is sufficient for most cases.\n");
        printf("-neg: Number of negative samples used for negative sampling. Default is 5.\n");
        printf("-neigh: Number of neighbors (K) in K-NNG, which is usually set as three times of perplexity. Default is 150.\n");
        printf("-gamma: The weights assigned to negative edges. Default is 7.\n");
        printf("-perp: The perplexity used for deciding edge weights in K-NNG. Default is 50.\n");
        return 0;
    }
    if ((i = ArgPos((char *)"-fea", argc, argv)) > 0) if_embed = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-input", argc, argv)) > 0) strcpy(infile, argv[i + 1]);
	if ((i = ArgPos((char *)"-output", argc, argv)) > 0) strcpy(outfile, argv[i + 1]);
	if ((i = ArgPos((char *)"-outdim", argc, argv)) > 0) out_dim = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-samples", argc, argv)) > 0) n_samples = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-threads", argc, argv)) > 0) n_threads = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-neg", argc, argv)) > 0) n_negative = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-neigh", argc, argv)) > 0) n_neighbors = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-trees", argc, argv)) > 0) n_trees = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-prop", argc, argv)) > 0) n_propagation = atoi(argv[i + 1]);
	if ((i = ArgPos((char *)"-alpha", argc, argv)) > 0) alpha = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-gamma", argc, argv)) > 0) n_gamma = atof(argv[i + 1]);
	if ((i = ArgPos((char *)"-perp", argc, argv)) > 0) perplexity = atof(argv[i + 1]);

	LargeVis model;
    if (if_embed)
        model.load_from_file(infile);
    else
        model.load_from_graph(infile);
	model.run(out_dim, n_threads, n_samples, n_propagation, alpha, n_trees, n_negative, n_neighbors, n_gamma, perplexity);

	model.save(outfile);
}
