#include "HFBBound.h"

/***************************************************************************/
void HFBBound(instance *inst)
/***************************************************************************/
{
	int n = inst->G->nnodes;
	int omega = inst->num_colors;  // number of independent sets in the coloring

	// For each vertex v, compute gamma[v]:
	// gamma[v] = 0.5 * sum of the (omega - 1) heaviest edge weights incident on v

	double *gamma = new double[n];

	for (int v = 0; v < n; v++)
	{
		int deg = inst->G->node_degree[v];

		// Collect the weights of all edges incident on v
		vector<double> incident_weights(deg);
		for (int k = 0; k < deg; k++)
		{
			int e = inst->G->adj_edge_idx[v][k];
			incident_weights[k] = inst->G->edge_weights[e];
		}

		// Sort in non-increasing order
		sort(incident_weights.begin(), incident_weights.end(), greater<double>());

		// Sum the top min(omega - 1, deg) weights
		int num_to_sum = min(omega - 1, deg);
		double sum = 0.0;
		for (int k = 0; k < num_to_sum; k++)
		{
			sum += incident_weights[k];
		}

		gamma[v] = 0.5 * sum;
	}

	// Select the omega vertices with the highest gamma values
	// Create index array and sort by gamma (non-increasing)
	vector<int> indices(n);
	for (int i = 0; i < n; i++)
	{
		indices[i] = i;
	}
	sort(indices.begin(), indices.end(), [&gamma](int a, int b) {
		return gamma[a] > gamma[b];
	});

	// Sum the gamma of the top omega vertices
	double bound = 0.0;
	for (int k = 0; k < omega; k++)
	{
		bound += gamma[indices[k]];
	}

	inst->HFBBound = bound;

	cout << "\nHFB Bound value: " << inst->HFBBound << endl;

	delete[] gamma;
}
