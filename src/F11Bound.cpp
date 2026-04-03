#include "F11Bound.h"
#include <chrono>

///////////////////////////////////////////////////////////////////////////////
#define DISABLE_CPLEX_OUTPUT_F11
///////////////////////////////////////////////////////////////////////////////


/***************************************************************************/
void F11Bound_Solve(instance *inst)
/***************************************************************************/
{
 try {
	int n = inst->G->nnodes;
	int m = inst->G->nedges;
	int k = inst->num_colors;   // upper bound on clique number

	// =====================================================================
	// Compact LP formulation F11 (Gouveia & Martins, 2015):
	//
	//   max  sum_{(i,j) in E} c_ij * y_ij
	//   s.t.
	//     y_ij <= x_i                       for all {i,j} in E
	//     y_ij <= x_j                       for all {i,j} in E
	//     x_i + x_j <= 1                    for all {i,j} NOT in E
	//     sum_{j in delta(i)} y_ij <= (k-1) * x_i   for all i in V
	//     sum_{i in V} x_i <= k
	//     x_i >= 0                          for all i in V
	//     y_ij >= 0                         for all {i,j} in E
	//
	// where k = num_colors (upper bound on clique number).
	// =====================================================================

	IloEnv env;
	IloModel model(env);

	// --- Variables ---
	// x_i for each vertex
	IloNumVarArray x(env, n, 0.0, IloInfinity, ILOFLOAT);

	// y_e for each edge e = (tail[e], head[e])
	IloNumVarArray y(env, m, 0.0, IloInfinity, ILOFLOAT);

	// --- Objective: max sum_{e} w_e * y_e ---
	IloExpr obj(env);
	for (int e = 0; e < m; e++)
		obj += inst->G->edge_weights[e] * y[e];
	model.add(IloMaximize(env, obj));
	obj.end();

	// --- Constraints ---

	// (1) y_ij <= x_i  (for tail of each edge)
	// (2) y_ij <= x_j  (for head of each edge)
	for (int e = 0; e < m; e++)
	{
		int i = inst->G->tail[e];
		int j = inst->G->head[e];
		model.add(y[e] - x[i] <= 0.0);
		model.add(y[e] - x[j] <= 0.0);
	}

	// (3) x_i + x_j <= 1  for all {i,j} NOT in E
	for (int i = 0; i < n; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			if (!inst->G->adj_matrix[i][j])
			{
				model.add(x[i] + x[j] <= 1.0);
			}
		}
	}

	if (!inst->PARAM_DISABLE_VALID_INEQ)
	{
		// (4) sum_{j in delta(i)} y_ij <= (k-1) * x_i   for all i in V
		// For each vertex i, iterate over its incident edges
		for (int i = 0; i < n; i++)
		{
			int deg = inst->G->node_degree[i];
			if (deg == 0) continue;

			IloExpr lhs(env);
			for (int idx = 0; idx < deg; idx++)
			{
				int edge_id = inst->G->adj_edge_idx[i][idx];
				lhs += y[edge_id];
			}
			model.add(lhs - (k - 1) * x[i] <= 0.0);
			lhs.end();
		}

		// (5) sum_{i in V} x_i <= k
		{
			IloExpr sum_x(env);
			for (int i = 0; i < n; i++)
				sum_x += x[i];
			model.add(sum_x <= (double)k);
			sum_x.end();
		}
	}

	// --- Solve with barrier, no crossover ---
	IloCplex cplex(model);

#ifdef DISABLE_CPLEX_OUTPUT_F11
	cplex.setOut(env.getNullStream());
#endif

	cplex.setParam(IloCplex::Param::Threads, 1);
	cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_BARRIER);
	cplex.setParam(IloCplex::Param::SolutionType, 2);   // no crossover
	cplex.setParam(IloCplex::Param::TimeLimit, inst->PARAM_TIME_LIMIT);

	bool time_limit_hit = false;

	try {
		cplex.solve();
	} catch (IloException &e) {
		cerr << "CPLEX solve exception: " << e.getMessage() << endl;
		time_limit_hit = true;
	}

	auto status = cplex.getStatus();
	if (status != IloAlgorithm::Optimal && status != IloAlgorithm::Feasible)
	{
		if (!inst->PARAM_DISABLE_VALID_INEQ) 
        { cout << "F11 Status: " << status << endl;}
        else
        { cout << "F1 Status: " << status << endl;}
		time_limit_hit = true;
	}

	// Store result
	try {
		double raw = cplex.getObjValue();
		// round() rather than floor(): barrier solvers routinely return x.9999... for
		// integer optima (e.g. sum_weights/2 for F1); floor would silently lose 1.
		// round() is a valid upper bound whenever edge weights are integers.
		inst->F11Bound = round(raw);
        if (!inst->PARAM_DISABLE_VALID_INEQ) 
        { cout << "F11 Bound value: " << inst->F11Bound << endl;}
        else
        { cout << "F1 Bound value: " << inst->F11Bound << endl;}

	} catch (...) {
		inst->F11Bound = -1.0;
	}
	inst->F11Bound_TimeLimitHit = time_limit_hit;

	// Cleanup
	cplex.end();
	model.end();
	env.end();

 } catch (IloException &e) {
    if(!inst->PARAM_DISABLE_VALID_INEQ) 
    { cerr << "CPLEX Exception in F11Bound_Solve: " << e.getMessage() << endl;}
    else
    {
	cerr << "CPLEX Exception in F1Bound_Solve: " << e.getMessage() << endl;
    }
	inst->F11Bound = -1.0;
	inst->F11Bound_TimeLimitHit = true;
 }
}
