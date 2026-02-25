#include "ShimizuBound.h"

/***************************************************************************/
// Helper: reorder stable sets according to the chosen sorting strategy and sense.
// Modifies inst->v_color in place.
/***************************************************************************/
static void reorder_stable_sets(instance *inst, const string &strategy, int sense)
{
	int k = inst->num_colors; // number of stable sets

	if (strategy == "size")
	{
		// Compute the size (number of vertices) of each stable set
		vector<int> ss_size(k, 0);
		for (int i = 0; i < inst->G->nnodes; i++)
			ss_size[inst->v_color[i]]++;

		// Sort stable set indices by size
		vector<int> order(k);
		for (int h = 0; h < k; h++) order[h] = h;

		sort(order.begin(), order.end(), [&](int a, int b) {
			return (sense > 0) ? (ss_size[a] < ss_size[b])
			                   : (ss_size[a] > ss_size[b]);
		});

		// Build mapping old_color -> new_color
		vector<int> new_color(k);
		for (int new_h = 0; new_h < k; new_h++)
			new_color[order[new_h]] = new_h;

		// Reassign vertex colors
		for (int i = 0; i < inst->G->nnodes; i++)
			inst->v_color[i] = new_color[inst->v_color[i]];
	}
	else if (strategy == "weight")
	{
		// Gamma score: for each vertex, gamma = sum of the (k-1) heaviest
		// incident edge weights (where k = number of stable sets).
		vector<double> gamma(inst->G->nnodes, 0.0);

		for (int i = 0; i < inst->G->nnodes; i++)
		{
			int deg = inst->G->node_degree[i];

			// Collect all incident edge weights
			vector<double> weights(deg);
			for (int j = 0; j < deg; j++)
			{
				int e = inst->G->adj_edge_idx[i][j];
				weights[j] = inst->G->edge_weights[e];
			}

			// Sort in descending order and sum the top min(deg, k-1)
			sort(weights.begin(), weights.end(), greater<double>());
			int top = min(deg, k - 1);
			for (int j = 0; j < top; j++)
				gamma[i] += weights[j];
		}

		// For each stable set, compute the sum of gamma values of its vertices
		vector<double> ss_weight(k, 0.0);
		for (int i = 0; i < inst->G->nnodes; i++)
			ss_weight[inst->v_color[i]] += gamma[i];

		// Sort stable set indices by weight
		vector<int> order(k);
		for (int h = 0; h < k; h++) order[h] = h;

		sort(order.begin(), order.end(), [&](int a, int b) {
			return (sense > 0) ? (ss_weight[a] < ss_weight[b])
			                   : (ss_weight[a] > ss_weight[b]);
		});

		// Build mapping old_color -> new_color
		vector<int> new_color(k);
		for (int new_h = 0; new_h < k; new_h++)
			new_color[order[new_h]] = new_h;

		// Reassign vertex colors
		for (int i = 0; i < inst->G->nnodes; i++)
			inst->v_color[i] = new_color[inst->v_color[i]];
	}
	// else: "natural" — keep the original ordering, do nothing
}


/***************************************************************************/
void ShimizuBound(instance *inst)
/***************************************************************************/
{
	// Reorder stable sets if a non-natural sorting strategy is requested
	if (inst->PARAM_SORTING_STRATEGY != "natural")
	{
		reorder_stable_sets(inst, inst->PARAM_SORTING_STRATEGY, inst->PARAM_SORTING_SENSE);
	}

	// Shimizu bound computation (Policy 2)
	inst->ShimizuBound = 0.0;
	int h_u, h_v, u, v;
	double **MIW; // MIW[i][h] = Weight of the edge of maximum weight linking node i to a node with color h

	double heaviest;

	MIW = new double *[inst->G->nnodes];

	for (int i = 0; i < inst->G->nnodes; i++)
	{
		MIW[i] = new double[inst->num_colors];
	}

	for (int i = 0; i < inst->G->nnodes; i++)
	{
		for (int h = 0; h < inst->num_colors; h++)
		{
			MIW[i][h] = 0.0;
		}
	}

	for (int e = 0; e < inst->G->nedges; e++)
	{
		u = inst->G->tail[e];
		v = inst->G->head[e];

		h_u = inst->v_color[u];
		h_v = inst->v_color[v];

		if (h_u == h_v)
			continue;

		if (h_u < h_v)
		{
			if (inst->G->edge_weights[e] > MIW[u][h_v])
			{
				MIW[u][h_v] = inst->G->edge_weights[e];
			}

			continue;
		}

		if (h_u > h_v)
		{
			if (inst->G->edge_weights[e] > MIW[v][h_u])
			{
				MIW[v][h_u] = inst->G->edge_weights[e];
			}

			continue;
		}
	}

	inst->Shimizu_w = new double[inst->G->nnodes];

	for (int i = 0; i < inst->G->nnodes; i++)
	{
		inst->Shimizu_w[i] = 0.0;
	}

	for (int i = 0; i < inst->G->nnodes; i++)
	{
		for (int h = inst->v_color[i]; h < inst->num_colors; h++)
		{
			inst->Shimizu_w[i] += MIW[i][h];
		}
	}

	for (int h = 0; h < inst->num_colors; h++)
	{
		heaviest = 0.0;
		for (int i = 0; i < inst->G->nnodes; i++)
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

	for (int i = 0; i < inst->G->nnodes; i++)
	{
		delete[] MIW[i];
	}
	delete[] MIW;
}


/***************************************************************************/
void ShimizuBound_first_policy(instance *inst)
/***************************************************************************/
{
	inst->ShimizuBound_first_policy = 0.0;
	int h_u, h_v, u, v;


	double heaviest;
	double *sigma = new double[inst->G->nnodes];


	for (int i = 0; i < inst->G->nnodes; i++)
	{
		sigma[i] = 0.0;
	}



	for (int e = 0; e < inst->G->nedges; e++)
	{
		u = inst->G->tail[e];
		v = inst->G->head[e];

		h_u = inst->v_color[u];
		h_v = inst->v_color[v];

		if (h_u == h_v)
			continue;

		else if (h_u < h_v)
		{
			sigma[u] += inst->G->edge_weights[e];
			continue;
		}

		else if (h_u > h_v)
		{
			sigma[v] += inst->G->edge_weights[e];
			continue;
		}
	}


	for (int h = 0; h < inst->num_colors; h++)
	{
		heaviest = 0.0;
		for (int i = 0; i < inst->G->nnodes; i++)
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
