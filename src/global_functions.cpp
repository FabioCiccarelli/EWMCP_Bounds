
#include "global_functions.h"

// #define print_MWCP_model
// #define print_MWCP_solution
// #define CPLEX_OUTPUT

/************************************************************************************************/
void generate_instances()
/************************************************************************************************/
{
	for(int n_nodes=10;n_nodes<=100;n_nodes+=10)
	{
		// for(int density=10;density<100;density+=10)
		for (int density = 91; density < 100; density += 1)
		{
			for (int seed = 1; seed <= 10; seed++)
			{
				srand(seed);

				int *heads = new int[10000000];
				int *tails = new int[10000000];
				int edges = 0;

				for (int i = 0; i < n_nodes; i++)
				{
					for (int j = i + 1; j < n_nodes; j++)
					{
						float random = (float)rand() / (float)RAND_MAX;
						if (random > (double)density / 100)
						{
							continue;
						}
						tails[edges] = i;
						heads[edges] = j;
						edges++;
					}
				}
				cout << n_nodes << "\t" << edges << "\t" << density << "\t" << (double)density / 100 << endl;

				// 1. Costruiamo il nome base usando uno stringstream (molto pulito)
				stringstream ss;
				ss << "../RANDOM/Instn" << n_nodes << "d" << density << "s" << seed;
				string base_name = ss.str();

				// 2. Primo file: istanza
				ofstream file_1(base_name);
				if (file_1.is_open()) {
					file_1 << "p edge\t" << n_nodes << "\t" << edges << "\n";
					for (int e = 0; e < edges; e++) {
						file_1 << "e\t" << tails[e] + 1 << "\t" << heads[e] + 1 << "\n";
					}
					file_1.close();
				}

				string weights_name = base_name + ".weights";
				ofstream file_2(weights_name);

				for (int e = 0; e < edges; e++)
				{
					int i = heads[e];
					int j = tails[e];

					file_2 << (((i+1)+(j+1))%(200))+1 << endl;
				}

				file_2.close();

				// ofstream info_NAMES("info_hard_randominstance.txt", ios::app);
				// info_NAMES
				// 	<< name << "\t"
				// 	<< dummy << "\t"
				// 	<< n_nodes << "\t"
				// 	<< edges << "\t"
				// 	<< density << "\t"
				// 	<< (double)edges / ((n_nodes * (n_nodes - 1)) / 2) << "\t"
				// 	<< "\n";
				// info_NAMES.close();

				delete[] heads;
				delete[] tails;
			}
		}
	}
	exit(-1);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
}

/************************************************************************************************/
void generate_edge_weights(instance *inst)
/************************************************************************************************/
{

	cout << "GENERATE EDGE WEIGHTS\n";
	for (int e = 0; e < inst->G->nedges; e++)
	{
		int i = inst->G->head[e];
		int j = inst->G->tail[e];
		inst->G->edge_weights[e] = (((i + 1) + (j + 1)) % (200)) + 1;

		cout << "H\t" << i << "\tT\t" << j << "\t weight \t" << inst->G->edge_weights[e] << endl;
	}

	cout << "DONE....\n\n";
}

/************************************************************************************************/
void write_edge_weights_file(instance *inst)
/************************************************************************************************/
{

	char dummy[1000];
	sprintf(dummy, "%s.weights", inst->istname_graph);

	ofstream info(dummy);

	for (int e = 0; e < inst->G->nedges; e++)
	{
		int i = inst->G->head[e];
		int j = inst->G->tail[e];

		info << (((i + 1) + (j + 1)) % (200)) + 1 << endl;
	}

	cout << "DONE....\n\n";
	info.close();
}

void dsatur_color(instance *inst)
{
	inst->v_color = new int[inst->G->nnodes];
	map<string, vector<string>> graph;

	for (int i = 0; i < inst->G->nnodes; i++)
	{

		string node = to_string(i);
		vector<string> neighbors;

		for (int k = 0; k < inst->G->node_degree[i]; k++)
		{
			neighbors.push_back(to_string(inst->G->adj_lists[i][k]));
		}

		graph[node] = neighbors;
	}

	Dsatur *algorithm = new Dsatur(graph);

	algorithm->color();

	inst->num_colors = algorithm->get_num_colors();

	for (int i = 0; i < inst->G->nnodes; i++)
	{
		inst->v_color[i] = algorithm->get_color(to_string(i));
	}

	// algorithm->print_chromatic();

	delete algorithm;
}

void random_color(instance *inst)
{
	inst->v_color = new int[inst->G->nnodes];

	// Visit each uncolored node in random order and assign to it the first color that is not used by its neighbors

	// Create a vector with all integers from 0 to n-1
	vector<int> indices(inst->G->nnodes);
	for (int i = 0; i < inst->G->nnodes; i++)
	{
		indices[i] = i;
	}
		
	// Set random seed and create random number generator;  
	mt19937 rng(inst-> PARAM_RANDOM_SEED);

	// Randomly shuffle the vertices indices with the chosen random seed
	shuffle(indices.begin(), indices.end(), rng);

	for(int i = 0; i < inst->G->nnodes; i++)
	{
		inst->v_color[i] = -1; // Initialize all colors to -1 (uncolored)
	}

	// Assign colors to vertices in the shuffled order
	inst->num_colors = 0;
	
	for (int i : indices)
	{

		// Create a boolean array to keep track of used colors
		vector<bool> used_colors(inst->G->nnodes, false);

		// Mark the colors used by all neighbors
		for (int k = 0; k < inst->G->node_degree[i]; k++)
		{
			int neighbor = inst->G->adj_lists[i][k];
			
			if (inst->v_color[neighbor] != -1) {
				used_colors[inst->v_color[neighbor]] = true;
			}
		}

		// Find the first available color
		int color = 0;
		while (used_colors[color]) {
			color++;
		}

		// Assign the color to vertex i
		inst->v_color[i] = color;

		// Update max_color if needed
		if (color > inst->num_colors) {
			inst->num_colors = color;
		}
		
	}

	inst->num_colors++; 
}
