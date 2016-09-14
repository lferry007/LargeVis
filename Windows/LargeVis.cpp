#include "LargeVis.h"
#include <map>

boost::minstd_rand LargeVis::generator(42u);
boost::uniform_real<> LargeVis::uni_dist(0, 1);
boost::variate_generator<boost::minstd_rand&, boost::uniform_real<> > LargeVis::uni(generator, uni_dist);

namespace boost
{
#ifdef BOOST_NO_EXCEPTIONS
	void throw_exception(std::exception const & e){
		throw 11; // or whatever
	};
#endif
}

LargeVis::LargeVis()
{
	vec = vis = prob = NULL;
	knn_vec = old_knn_vec = NULL;
	annoy_index = NULL;
	head = alias = NULL;
    neg_table = NULL;
}

void LargeVis::clean_model()
{
	if (vis) delete[] vis;
	if (prob) delete[] prob;
	if (knn_vec) delete[] knn_vec;
	if (old_knn_vec) delete[] old_knn_vec;
	if (annoy_index) delete annoy_index;
	if (neg_table) delete[] neg_table;
	if (alias) delete[] alias;
	vis = prob = NULL;
	knn_vec = old_knn_vec = NULL;
	annoy_index = NULL;
    neg_table = NULL;
    alias = NULL;

	edge_count_actual = 0;
	neg_size = 1e8;
}

void LargeVis::clean_graph()
{
	if (head) { delete[] head; head = NULL; }

	n_edge = 0;
	next.clear(); edge_from.clear(); edge_to.clear(); reverse.clear(); edge_weight.clear(); names.clear();
}

void LargeVis::clean_data()
{
	if (vec) {delete[] vec; vec = NULL;	}
	clean_graph();
}

void LargeVis::load_from_file(char *infile)
{
	clean_data();
	FILE *fin = fopen(infile, "rb");
	if (fin == NULL)
	{
		printf("\nFile not found!\n");
		return;
	}
    printf("Reading input file %s ......", infile); fflush(stdout);
	fscanf(fin, "%lld%lld", &n_vertices, &n_dim);
	vec = new real[n_vertices * n_dim];
	for (long long i = 0; i < n_vertices; ++i)
	{
		for (long long j = 0; j < n_dim; ++j)
		{
			fscanf(fin, "%f", &vec[i * n_dim + j]);
		}
	}
	fclose(fin);
	printf(" Done.\n");
	printf("Total vertices : %lld\tDimension : %lld\n", n_vertices, n_dim);
}

void LargeVis::load_from_data(real *data, long long n_vert, long long n_di)
{
	clean_data();
	vec = data;
	n_vertices = n_vert;
	n_dim = n_di;
	printf("Total vertices : %lld\tDimension : %lld\n", n_vertices, n_dim);
}

void LargeVis::load_from_graph(char *infile)
{
	clean_data();
	char *w1 = new char[1000];
	char *w2 = new char[10000];
	long long x, y, i, p;
	real weight;
	std::map<std::string, long long> dict;
	n_vertices = 0;
	FILE *fin = fopen(infile, "rb");
	if (fin == NULL)
	{
		printf("\nFile not found!\n");
		return;
	}
	printf("Reading input file %s ......%c", infile, 13);
	while (fscanf(fin, "%s%s%f", w1, w2, &weight) == 3)
	{
		if (!dict.count(w1)) { dict[w1] = n_vertices++; names.push_back(w1); }
		if (!dict.count(w2)) { dict[w2] = n_vertices++; names.push_back(w2); }
		x = dict[w1];
		y = dict[w2];
		edge_from.push_back(x);
		edge_to.push_back(y);
		edge_weight.push_back(weight);
		next.push_back(-1);
		++n_edge;
		if (n_edge % 5000 == 0)
		{
			printf("Reading input file %s ...... %lldK edges%c", infile, n_edge / 1000, 13);
			fflush(stdout);
		}
	}
	fclose(fin);
	delete[] w1;
	delete[] w2;
	head = new long long[n_vertices];
	for (i = 0; i < n_vertices; ++i) head[i] = -1;
	for (p = 0; p < n_edge; ++p)
	{
		x = edge_from[p];
		next[p] = head[x];
		head[x] = p;
	}
	printf("\nTotal vertices : %lld\tTotal edges : %lld\n", n_vertices, n_edge);
}

void LargeVis::save(char *outfile)
{
	FILE *fout = fopen(outfile, "wb");
	fprintf(fout, "%lld %lld\n", n_vertices, out_dim);
	for (long long i = 0; i < n_vertices; ++i)
	{
		if(names.size()) fprintf(fout, "%s ", names[i].c_str());
		for (long long j = 0; j < out_dim; ++j)
		{
			if (j) fprintf(fout, " ");
			fprintf(fout, "%.6f", vis[i * out_dim + j]);
		}
		fprintf(fout, "\n");
	}
	fclose(fout);
}

real *LargeVis::get_ans()
{
	return vis;
}

long long LargeVis::get_n_vertices()
{
	return n_vertices;
}

long long LargeVis::get_out_dim()
{
	return out_dim;
}

void LargeVis::normalize()
{
    printf("Normalizing ......"); fflush(stdout);
	real *mean = new real[n_dim];
	for (long long i = 0; i < n_dim; ++i) mean[i] = 0;
	for (long long i = 0, ll = 0; i < n_vertices; ++i, ll += n_dim)
	{
		for (long long j = 0; j < n_dim; ++j)
			mean[j] += vec[ll + j];
	}
	for (long long j = 0; j < n_dim; ++j)
		mean[j] /= n_vertices;
	real mX = 0;
	for (long long i = 0, ll = 0; i < n_vertices; ++i, ll += n_dim)
	{
		for (long long j = 0; j < n_dim; ++j)
		{
			vec[ll + j] -= mean[j];
			if (fabs(vec[ll + j]) > mX)	mX = fabs(vec[ll + j]);
		}
	}
	for (long long i = 0; i < n_vertices * n_dim; ++i)
		vec[i] /= mX;
	delete[] mean;
	printf(" Done.\n");
}

real LargeVis::CalcDist(long long x, long long y)
{
	real ret = 0;
	long long i, lx = x * n_dim, ly = y * n_dim;
	for (i = 0; i < n_dim; ++i)
		ret += (vec[lx + i] - vec[ly + i]) * (vec[lx + i] - vec[ly + i]);
	return ret;
}

void LargeVis::init_alias_table()
{
	alias = new long long[n_edge];
	prob = new real[n_edge];

	real *norm_prob = new real[n_edge];
	long long *large_block = new long long[n_edge];
	long long *small_block = new long long[n_edge];

	real sum = 0;
	long long cur_small_block, cur_large_block;
	long long num_small_block = 0, num_large_block = 0;

	for (long long k = 0; k < n_edge; ++k) sum += edge_weight[k];
	for (long long k = 0; k < n_edge; ++k) norm_prob[k] = edge_weight[k] * n_edge / sum;

	for (long long k = n_edge - 1; k >= 0; --k)
	{
		if (norm_prob[k] < 1)
			small_block[num_small_block++] = k;
		else
			large_block[num_large_block++] = k;
	}

	while (num_small_block && num_large_block)
	{
		cur_small_block = small_block[--num_small_block];
		cur_large_block = large_block[--num_large_block];
		prob[cur_small_block] = norm_prob[cur_small_block];
		alias[cur_small_block] = cur_large_block;
		norm_prob[cur_large_block] = norm_prob[cur_large_block] + norm_prob[cur_small_block] - 1;
		if (norm_prob[cur_large_block] < 1)
			small_block[num_small_block++] = cur_large_block;
		else
			large_block[num_large_block++] = cur_large_block;
	}

	while (num_large_block) prob[large_block[--num_large_block]] = 1;
	while (num_small_block) prob[small_block[--num_small_block]] = 1;

	delete[] norm_prob;
	delete[] small_block;
	delete[] large_block;
}

long long LargeVis::sample_an_edge(real rand_value1, real rand_value2)
{
	long long k = (long long)((n_edge - 0.1) * rand_value1);
	return rand_value2 <= prob[k] ? k : alias[k];
}

void LargeVis::annoy_thread(int id)
{
	long long lo = id * n_vertices / n_threads;
	long long hi = (id + 1) * n_vertices / n_threads;
	AnnoyIndex<int, real, Euclidean, Kiss64Random> *cur_annoy_index = NULL;
	if (id > 0)
	{
		cur_annoy_index = new AnnoyIndex<int, real, Euclidean, Kiss64Random>(n_dim);
		cur_annoy_index->load("annoy_index_file");
	}else
		cur_annoy_index = annoy_index;
	for (long long i = lo; i < hi; ++i)
	{
		cur_annoy_index->get_nns_by_item(i, n_neighbors + 1, (n_neighbors + 1) * n_trees, &knn_vec[i], NULL);
		for (long long j = 0; j < knn_vec[i].size(); ++i)
			if (knn_vec[i][j] == i)
			{
				knn_vec[i].erase(knn_vec[i].begin() + j);
				break;
			}
	}
	if (id > 0) delete cur_annoy_index;
}

void LargeVis::run_annoy()
{
    printf("Running ANNOY ......"); fflush(stdout);
	annoy_index = new AnnoyIndex<int, real, Euclidean, Kiss64Random>(n_dim);
	for (long long i = 0; i < n_vertices; ++i)
		annoy_index->add_item(i, &vec[i * n_dim]);
	annoy_index->build(n_trees);
	if(n_threads > 1) annoy_index->save("annoy_index_file");
	knn_vec = new std::vector<int>[n_vertices];

	boost::thread *pt = new boost::thread[n_threads];
	for (int i = 0; i < n_threads; ++i)
		pt[i] = boost::thread(&LargeVis::annoy_thread, this, i);
	for (int i = 0; i < n_threads; ++i)
		pt[i].join();
	delete[] pt;
    delete annoy_index; annoy_index = NULL;
	printf(" Done.\n");
}

void LargeVis::propagation_thread(int id)
{
	long long lo = id * n_vertices / n_threads;
	long long hi = (id + 1) * n_vertices / n_threads;
	int *check = new int[n_vertices];
	std::priority_queue< pair<real, int> > heap;
	long long x, y, i, j, l1, l2;
	for (x = 0; x < n_vertices; ++x) check[x] = -1;
	for (x = lo; x < hi; ++x)
	{
		check[x] = x;
		std::vector<int> &v1 = old_knn_vec[x];
		l1 = v1.size();
		for (i = 0; i < l1; ++i)
		{
			y = v1[i];
			check[y] = x;
			heap.push(std::make_pair(CalcDist(x, y), y));
			if (heap.size() == n_neighbors + 1) heap.pop();
		}
		for (i = 0; i < l1; ++i)
		{
			std::vector<int> &v2 = old_knn_vec[v1[i]];
			l2 = v2.size();
			for (j = 0; j < l2; ++j) if (check[y = v2[j]] != x)
			{
				check[y] = x;
				heap.push(std::make_pair(CalcDist(x, y), (int)y));
				if (heap.size() == n_neighbors + 1) heap.pop();
			}
		}
		while (!heap.empty())
		{
			knn_vec[x].push_back(heap.top().second);
			heap.pop();
		}
	}
	delete[] check;
}

void LargeVis::run_propagation()
{
	for (int i = 0; i < n_propagations; ++i)
	{
		printf("Running propagation %d/%d%c", i + 1, n_propagations, 13);
		fflush(stdout);
		old_knn_vec = knn_vec;
		knn_vec = new std::vector<int>[n_vertices];
		boost::thread *pt = new boost::thread[n_threads];
		for (int j = 0; j < n_threads; ++j) pt[j] = boost::thread(&LargeVis::propagation_thread, this, j);
		for (int j = 0; j < n_threads; ++j) pt[j].join();
		delete[] pt;
		delete[] old_knn_vec; old_knn_vec = NULL;
	}
	printf("\n");
}

void LargeVis::compute_similarity_thread(int id)
{
	long long lo = id * n_vertices / n_threads;
	long long hi = (id + 1) * n_vertices / n_threads;
	long long x, iter, p;
	real beta, lo_beta, hi_beta, sum_weight, H, tmp;
	for (x = lo; x < hi; ++x)
	{
		beta = 1;
		lo_beta = hi_beta = -1;
		for (iter = 0; iter < 200; ++iter)
		{
			H = sum_weight = 0;
			for (p = head[x]; p >= 0; p = next[p])
			{
				sum_weight += tmp = exp(-beta * edge_weight[p]);
				H += beta * (edge_weight[p] * tmp);
			}
			H = (H / sum_weight) + log(sum_weight);
			if (fabs(H - log(perplexity)) < 1e-5) break;
			if (H > log(perplexity))
			{
				lo_beta = beta;
				if (hi_beta < 0) beta *= 2; else beta = (beta + hi_beta) / 2;
			}
			else{
				hi_beta = beta;
				if (lo_beta < 0) beta /= 2; else beta = (lo_beta + beta) / 2;
			}
		}
		for (p = head[x], sum_weight = 0; p >= 0; p = next[p])
		{
			sum_weight += edge_weight[p] = exp(-beta * edge_weight[p]);
		}
		for (p = head[x]; p >= 0; p = next[p])
		{
			edge_weight[p] /= sum_weight;
		}
	}
}

void LargeVis::search_reverse_thread(int id)
{
	long long lo = id * n_vertices / n_threads;
	long long hi = (id + 1) * n_vertices / n_threads;
	long long x, y, p, q;
	for (x = lo; x < hi; ++x)
	{
		for (p = head[x]; p >= 0; p = next[p])
		{
			y = edge_to[p];
			for (q = head[y]; q >= 0; q = next[q])
			{
				if (edge_to[q] == x) break;
			}
			reverse[p] = q;
		}
	}
}

void LargeVis::compute_similarity()
{
    printf("Computing similarities ......"); fflush(stdout);
	n_edge = 0;
	head = new long long[n_vertices];
	long long i, x, y, p, q;
	real sum_weight = 0;
	for (i = 0; i < n_vertices; ++i) head[i] = -1;
	for (x = 0; x < n_vertices; ++x)
	{
		for (i = 0; i < knn_vec[x].size(); ++i)
		{
			edge_from.push_back((int)x);
			edge_to.push_back((int)(y = knn_vec[x][i]));
			edge_weight.push_back(CalcDist(x, y));
			next.push_back(head[x]);
			reverse.push_back(-1);
			head[x] = n_edge++;
		}
	}
    delete[] vec; vec = NULL;
    delete[] knn_vec; knn_vec = NULL;
	boost::thread *pt = new boost::thread[n_threads];
	for (int j = 0; j < n_threads; ++j) pt[j] = boost::thread(&LargeVis::compute_similarity_thread, this, j);
	for (int j = 0; j < n_threads; ++j) pt[j].join();
	delete[] pt;

	pt = new boost::thread[n_threads];
	for (int j = 0; j < n_threads; ++j) pt[j] = boost::thread(&LargeVis::search_reverse_thread, this, j);
	for (int j = 0; j < n_threads; ++j) pt[j].join();
	delete[] pt;
	
	for (x = 0; x < n_vertices; ++x)
	{
		for (p = head[x]; p >= 0; p = next[p])
		{
			y = edge_to[p];
			q = reverse[p];
			if (q == -1)
			{
				edge_from.push_back((int)y);
				edge_to.push_back((int)x);
				edge_weight.push_back(0);
				next.push_back(head[y]);
				reverse.push_back(p);
				q = reverse[p] = head[y] = n_edge++;
			}
			if (x > y){
				sum_weight += edge_weight[p] + edge_weight[q];
				edge_weight[p] = edge_weight[q] = (edge_weight[p] + edge_weight[q]) / 2;
			}
		}
	}
	printf(" Done.\n");
}

void LargeVis::test_accuracy()
{
	long long test_case = 100;
	std::priority_queue< pair<real, int> > *heap = new std::priority_queue< pair<real, int> >;
	long long hit_case = 0, i, j, x, y;
	for (i = 0; i < test_case; ++i)
	{
		x = floor(uni() * (n_vertices - 0.1));
		for (y = 0; y < n_vertices; ++y) if (x != y)
		{
			heap->push(std::make_pair(CalcDist(x, y), y));
			if (heap->size() == n_neighbors + 1) heap->pop();
		}
		while (!heap->empty())
		{
			y = heap->top().second;
			heap->pop();
			for (j = 0; j < knn_vec[x].size(); ++j) if (knn_vec[x][j] == y)
				++hit_case;
		}
	}
    delete heap;
	printf("Test knn accuracy : %.2f%%\n", hit_case * 100.0 / (test_case * n_neighbors));
}

void LargeVis::construt_knn()
{
	normalize();
	run_annoy();
	run_propagation();
	test_accuracy();
	compute_similarity();

	FILE *fout = fopen("knn_graph.txt", "wb");
	for (long long p = 0; p < n_edge; ++p)
	{
		fprintf(fout, "%lld %lld %.6f\n", edge_from[p], edge_to[p], edge_weight[p]);
	}
	fclose(fout);
}

void LargeVis::init_neg_table()
{
	long long x, p, i;
    reverse.clear(); vector<long long> (reverse).swap(reverse);
	real sum_weights = 0, dd, *weights = new real[n_vertices];
	for (i = 0; i < n_vertices; ++i) weights[i] = 0;
	for (x = 0; x < n_vertices; ++x)
	{
		for (p = head[x]; p >= 0; p = next[p])
		{
			weights[x] += edge_weight[p];
		}
		sum_weights += weights[x] = pow(weights[x], 0.75);
	}
    next.clear(); vector<long long> (next).swap(next);
    delete[] head; head = NULL;
	neg_table = new int[neg_size];
	dd = weights[0];
	for (i = x = 0; i < neg_size; ++i)
	{
		neg_table[i] = x;
		if (i / (real)neg_size > dd / sum_weights && x < n_vertices - 1)
		{
			dd += weights[++x];
		}
	}
	delete[] weights;
}

void LargeVis::visualize_thread(int id)
{
	long long edge_count = 0, last_edge_count = 0;
	long long x, y, p, lx, ly, i, j;
	real f, g, gg, cur_alpha = initial_alpha;
	real *cur = new real[out_dim];
	real *err = new real[out_dim];
	real grad_clip = 5.0;
	while (1)
	{
		if (edge_count > n_samples / n_threads + 2) break;
		if (edge_count - last_edge_count > 10000)
		{
			edge_count_actual += edge_count - last_edge_count;
			last_edge_count = edge_count;
			cur_alpha = initial_alpha * (1 - edge_count_actual / (n_samples + 1.0));
			if (cur_alpha < initial_alpha * 0.0001) cur_alpha = initial_alpha * 0.0001;
			printf("%cFitting model\tAlpha: %f Progress: %.3lf%%", 13, cur_alpha, (real)edge_count_actual / (real)(n_samples + 1) * 100);
			fflush(stdout);
		}
		p = sample_an_edge(uni(), uni());
		x = edge_from[p];
		y = edge_to[p];
		lx = x * out_dim;
		for (i = 0; i < out_dim; ++i) cur[i] = vis[lx + i], err[i] = 0;
		for (i = 0; i < n_negatives + 1; ++i)
		{
			if (i > 0)
			{
				y = neg_table[(unsigned long long)floor(uni() * (neg_size - 0.1))];
				if (y == edge_to[p]) continue;
			}
			ly = y * out_dim;
			for (j = 0, f= 0; j < out_dim; ++j) f += (cur[j] - vis[ly + j]) * (cur[j] - vis[ly + j]);
			if (i == 0) g = -2 / (1 + f);
			else g = 2 * gamma / (1 + f) / (0.1 + f);
			for (j = 0; j < out_dim; ++j)
			{
				gg = g * (cur[j] - vis[ly + j]);
				if (gg > grad_clip) gg = grad_clip;
				if (gg < -grad_clip) gg = -grad_clip;
				err[j] += gg * cur_alpha;
			
				gg = g * (vis[ly + j] - cur[j]);
				if (gg > grad_clip) gg = grad_clip;
				if (gg < -grad_clip) gg = -grad_clip;
				vis[ly + j] += gg * cur_alpha;
			}
		}
		for (int j = 0; j < out_dim; ++j) vis[lx + j] += err[j];
		++edge_count;
	}
	delete[] cur;
	delete[] err;
}

void LargeVis::visualize()
{
	long long i;
	vis = new real[n_vertices * out_dim];
	for (i = 0; i < n_vertices * out_dim; ++i) vis[i] = (uni() - 0.5) / out_dim * 0.0001;
	init_neg_table();
	init_alias_table();
	boost::thread *pt = new boost::thread[n_threads];
	for (int j = 0; j < n_threads; ++j) pt[j] = boost::thread(&LargeVis::visualize_thread, this, j);
	for (int j = 0; j < n_threads; ++j) pt[j].join();
	delete[] pt;
	printf("\n");
}

void LargeVis::run(long long out_d, long long n_thre, long long n_samp, long long n_prop, real alph, long long n_tree, long long n_nega, long long n_neig, real gamm, real perp)
{
	clean_model();
	if (!vec && !head)
	{
		printf("Missing training data!\n");
		return;
	}
	out_dim = out_d < 0 ? 2 : out_d;
	initial_alpha = alph < 0 ? 1.0 : alph;
	n_threads = n_thre < 0 ? 8 : n_thre;
	n_samples = n_samp;
	n_negatives = n_nega < 0 ? 5 : n_nega;
	n_neighbors = n_neig < 0 ? 150 : n_neig;
	n_trees = n_tree;
	n_propagations = n_prop < 0 ? 3 : n_prop;
	gamma = gamm < 0 ? 7.0 : gamm;
	perplexity = perp < 0 ? 50.0 : perp;
	if (n_samples < 0)
	{
		if (n_vertices < 10000)
			n_samples = 1000;
		else if (n_vertices < 1000000)
			n_samples = (n_vertices - 10000) * 9000 / (1000000 - 10000) + 1000;
		else n_samples = n_vertices / 100;
	}
	n_samples *= 1000000;
	if (n_trees < 0)
	{
		if (n_vertices < 100000)
			n_trees = 10;
		else if (n_vertices < 1000000)
			n_trees = 20;
		else if (n_vertices < 5000000)
			n_trees = 50;
		else n_trees = 100;
	}
	if (vec) { clean_graph(); construt_knn(); }
	visualize();
}
