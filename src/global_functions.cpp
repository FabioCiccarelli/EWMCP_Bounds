
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

				char name[1000];

				sprintf(name, "../RANDOM/Instn%dd%ds%d", n_nodes, density, seed);

				ofstream file_1(name);

				file_1 << "p edge\t" << n_nodes << "\t" << edges << endl;

				for (int e = 0; e < edges; e++)
				{
					file_1 << "e\t" << tails[e] + 1 << "\t" << heads[e] + 1 << endl;
				}

				file_1.close();

				char dummy[1000];
				sprintf(dummy, "%s.weights", name);

				ofstream file_2(dummy);

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
	for (int e = 0; e < inst->G->m; e++)
	{
		int i = inst->G->H[e];
		int j = inst->G->T[e];
		inst->G->P[e] = (((i + 1) + (j + 1)) % (200)) + 1;

		cout << "H\t" << i << "\tT\t" << j << "\t weight \t" << inst->G->P[e] << endl;
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

	for (int e = 0; e < inst->G->m; e++)
	{
		int i = inst->G->H[e];
		int j = inst->G->T[e];

		info << (((i + 1) + (j + 1)) % (200)) + 1 << endl;
	}

	cout << "DONE....\n\n";
	info.close();
}

void ShimizuBound(instance *inst)
{
	inst->ShimizuBound = 0.0;
	int h_u, h_v, u, v;
	double **MIW; // MIW[i][h] = Weight of the edge of maximum weight linking node i to a node with color h

	double heaviest;

	MIW = new double *[inst->G->n];

	for (int i = 0; i < inst->G->n; i++)
	{
		MIW[i] = new double[inst->num_colors];
	}

	for (int i = 0; i < inst->G->n; i++)
	{
		for (int h = 0; h < inst->num_colors; h++)
		{
			MIW[i][h] = 0.0;
		}
	}

	for (int e = 0; e < inst->G->m; e++)
	{
		u = inst->G->T[e];
		v = inst->G->H[e];

		h_u = inst->v_color[u];
		h_v = inst->v_color[v];

		if (h_u == h_v)
			continue;

		if (h_u < h_v)
		{
			if (inst->G->P[e] > MIW[u][h_v])
			{
				MIW[u][h_v] = inst->G->P[e];
			}

			continue;
		}

		if (h_u > h_v)
		{
			if (inst->G->P[e] > MIW[v][h_u])
			{
				MIW[v][h_u] = inst->G->P[e];
			}

			continue;
		}
	}

	inst->Shimizu_w = new double[inst->G->n];

	for (int i = 0; i < inst->G->n; i++)
	{
		inst->Shimizu_w[i] = 0.0;
	}

	for (int i = 0; i < inst->G->n; i++)
	{
		for (int h = inst->v_color[i]; h < inst->num_colors; h++)
		{
			inst->Shimizu_w[i] += MIW[i][h];
		}
	}

	for (int h = 0; h < inst->num_colors; h++)
	{
		heaviest = 0.0;
		for (int i = 0; i < inst->G->n; i++)
		{
			if (inst->v_color[i] == h)
			{
				if (inst->Shimizu_w[i] > heaviest)
				{
					heaviest = inst->Shimizu_w[i];
				}
			}
		}

		inst->ShimizuBound += heaviest;
	}

	cout << "\nShimizu Bound value: " << inst->ShimizuBound << endl;

	for (int i = 0; i < inst->G->n; i++)
	{
		delete[] MIW[i];
	}
	delete[] MIW;
}


void ShimizuBound_first_policy(instance *inst)
{
	inst->ShimizuBound_first_policy = 0.0;
	int h_u, h_v, u, v;


	double heaviest;
	double *sigma = new double[inst->G->n];


	for (int i = 0; i < inst->G->n; i++)
	{
		sigma[i] = 0.0;
	}



	for (int e = 0; e < inst->G->m; e++)
	{
		u = inst->G->T[e];
		v = inst->G->H[e];

		h_u = inst->v_color[u];
		h_v = inst->v_color[v];

		if (h_u == h_v)
			continue;

		else if (h_u < h_v)
		{
			sigma[u] += inst->G->P[e];
			continue;
		}

		else if (h_u > h_v)
		{
			sigma[v] += inst->G->P[e];
			continue;
		}
	}


	for (int h = 0; h < inst->num_colors; h++)
	{
		heaviest = 0.0;
		for (int i = 0; i < inst->G->n; i++)
		{
			if (inst->v_color[i] == h)
			{
				if (sigma[i] > heaviest)
				{
					heaviest = sigma[i];
				}
			}
		}

		inst->ShimizuBound_first_policy += heaviest;
	}

	// cout << "\n\nShimizu Bound (first policy) value: " << inst->ShimizuBound_first_policy << "\n\n" << endl;


	delete[] sigma;
}

void dsatur_color(instance *inst)
{
	inst->v_color = new int[inst->G->n];
	map<string, vector<string>> graph;

	for (int i = 0; i < inst->G->n; i++)
	{

		string node = to_string(i);
		vector<string> neighbors;

		for (int j = 0; j < inst->G->n; j++)
		{
			if (inst->G->AMatrix[i][j] == 1 || inst->G->AMatrix[j][i] == 1)
			{
				neighbors.push_back(to_string(j));
			}
		}

		graph[node] = neighbors;
	}

	Dsatur *algorithm = new Dsatur(graph);

	algorithm->color();

	inst->num_colors = algorithm->get_num_colors();

	for (int i = 0; i < inst->G->n; i++)
	{
		inst->v_color[i] = algorithm->get_color(to_string(i));
	}

	// algorithm->print_chromatic();

	delete algorithm;
}

void random_color(instance *inst)
{
	inst->v_color = new int[inst->G->n];

	// Visit each uncolored node in random order and assign to it the first color that is not used by its neighbors

	// Create a vector with all integers from 0 to n-1
	vector<int> indices(inst->G->n);
	for (int i = 0; i < inst->G->n; i++)
	{
		indices[i] = i;
	}
		
	// Set random seed and create random number generator;  
	mt19937 rng(inst-> PARAM_RANDOM_SEED);

	// Randomly shuffle the vertices indices with the chosen random seed
	shuffle(indices.begin(), indices.end(), rng);

	for(int i = 0; i < inst->G->n; i++)
	{
		inst->v_color[i] = -1; // Initialize all colors to -1 (uncolored)
	}

	// Assign colors to vertices in the shuffled order
	inst->num_colors = 0;
	
	for (int i : indices)
	{

		// Create a boolean array to keep track of used colors
		vector<bool> used_colors(inst->G->n, false);

		// Mark the colors used by forward neighbors
		for (int k = inst->G->NFS[i]; k < inst->G->NFS[i+1]; k++) 
		{
			int e = inst->G->AFS[k];
			int neighbor = inst->G->H[e];
			
			if (inst->v_color[neighbor] != -1) {
				used_colors[inst->v_color[neighbor]] = true;
			}
		}

		// Mark the colors used by backward neighbors
		for (int k = inst->G->NBS[i]; k < inst->G->NBS[i+1]; k++) 
		{
			int e = inst->G->ABS[k];
			int neighbor = inst->G->T[e];
			
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
