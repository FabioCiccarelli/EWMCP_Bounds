#include "ShimizuBound.h"

/***************************************************************************/
void ShimizuBound(instance *inst, bool smartSorting)
/***************************************************************************/
{
	// If smartSorting is enabled, reorder stable sets so that those with
	// higher total incident-edge weight are assigned higher color indices.
	if (smartSorting)
	{
		// 1. Compute the star weight of each vertex: sum of all incident edge weights
		vector<double> node_star_weight(inst->G->nnodes, 0.0);
		for (int i = 0; i < inst->G->nnodes; i++)
		{
			for (int k = 0; k < inst->G->node_degree[i]; k++)
			{
				int e = inst->G->adj_edge_idx[i][k];
				node_star_weight[i] += inst->G->edge_weights[e];
			}
		}

		// 2. Compute the weight of each stable set (color class):
		//    weight(S_h) = sum of star weights of vertices with color h
		vector<double> ss_weight(inst->num_colors, 0.0);
		for (int i = 0; i < inst->G->nnodes; i++)
		{
			ss_weight[inst->v_color[i]] += node_star_weight[i];
		}

		// 3. Sort stable sets by weight in ascending order (heaviest last)
		vector<int> order(inst->num_colors);
		for (int h = 0; h < inst->num_colors; h++) order[h] = h;
		sort(order.begin(), order.end(), [&](int a, int b) {
			return ss_weight[a] < ss_weight[b];
		});

		// 4. Build the mapping: old_color -> new_color
		vector<int> new_color(inst->num_colors);
		for (int new_h = 0; new_h < inst->num_colors; new_h++)
		{
			new_color[order[new_h]] = new_h;
		}

		// 5. Reassign vertex colors
		for (int i = 0; i < inst->G->nnodes; i++)
		{
			inst->v_color[i] = new_color[inst->v_color[i]];
		}
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
