#ifndef LARGEVIS_H
#define LARGEVIS_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <vector>

#include "ANNOY/annoylib.h"
#include "ANNOY/kissrandom.h"

#include <pthread.h>
#include <gsl/gsl_rng.h>

typedef float real;

struct arg_struct{
	void *ptr;
	int id;
	arg_struct(void *x, int y) :ptr(x), id(y){}
};

class LargeVis{
private:
	long long n_vertices, n_dim, out_dim, n_samples, n_threads, n_negatives, n_neighbors, n_trees, n_propagations, edge_count_actual;
	real initial_alpha, gamma, perplexity;
	real *vec, *vis;
	std::vector<string> names;
	std::vector<int> *knn_vec, *old_knn_vec;
	AnnoyIndex<int, real, Euclidean, Kiss64Random> *annoy_index;
	long long n_edge, *head;
    std::vector<long long> next, reverse;
    std::vector<int> edge_from, edge_to;
	std::vector<real> edge_weight;
    int *neg_table;
    long long neg_size;
	long long *alias;
	real *prob;
	static const gsl_rng_type * gsl_T;
	static gsl_rng * gsl_r;

	void clean_model();
	void clean_data();
	void clean_graph();
	void normalize();
	real CalcDist(long long x, long long y);
	void init_alias_table();
	long long sample_an_edge(real rand_value1, real rand_value2);
	void run_annoy();
	void annoy_thread(int id);
	static void *annoy_thread_caller(void *arg);
	void run_propagation();
	void propagation_thread(int id);
	static void *propagation_thread_caller(void *arg);
	void test_accuracy();
	void compute_similarity();
	void compute_similarity_thread(int id);
	static void *compute_similarity_thread_caller(void *arg);
	void search_reverse_thread(int id);
	static void *search_reverse_thread_caller(void *arg);
	void construt_knn();
	void init_neg_table();
	void visualize_thread(int id);
	static void *visualize_thread_caller(void *arg);
	void visualize();
public:
	LargeVis();
	void load_from_file(char *infile);
	void load_from_graph(char *infile, bool use_default_weight = false);
	void load_from_data(real *data, long long n_vert, long long n_di);
	void save(char *outfile);
	void run(long long out_d = -1, long long n_thre = -1, long long n_samp = -1, long long n_prop = -1, real alph = -1, long long n_tree = -1, long long n_nega = -1, long long n_neig = -1, real gamm = -1, real perp = -1);
	real *get_ans();
	long long get_n_vertices();
	long long get_out_dim();
};

#endif