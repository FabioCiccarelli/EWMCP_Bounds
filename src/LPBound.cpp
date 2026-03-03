#include "LPBound.h"
#include "TSM-mod.h"
#include <chrono>

///////////////////////////////////////////////////////////////////////////////
#define DISABLE_CPLEX_OUTPUT_LP
// #define VERIFY_MWIS_WITH_CPLEX  // Compare TSM vs CPLEX MWIS to verify optimality
///////////////////////////////////////////////////////////////////////////////

#define SEPARATION_SCALE 100000  // Scale factor for MWIS separation weights


/***************************************************************************/
// Build the complement graph of inst->G.
// Returns a newly allocated Graph* representing the complement.
/***************************************************************************/
static Graph* build_complement_graph(Graph *G)
{
	int n = G->nnodes;

	// Count complement edges: C(n,2) - |E|
	int max_edges = n * (n - 1) / 2;
	int complement_nedges = max_edges - G->nedges;

	int *comp_tails = new int[complement_nedges];
	int *comp_heads = new int[complement_nedges];

	int idx = 0;
	for (int i = 0; i < n; i++)
	{
		for (int j = i + 1; j < n; j++)
		{
			if (!G->adj_matrix[i][j])
			{
				comp_tails[idx] = i;
				comp_heads[idx] = j;
				idx++;
			}
		}
	}
	assert(idx == complement_nedges);

	Graph *comp = new Graph(n, complement_nedges, comp_tails, comp_heads);

	delete[] comp_tails;
	delete[] comp_heads;

	return comp;
}


/***************************************************************************/
// Solve the Maximum Weighted Independent Set problem using CPLEX.
// Returns the optimal weight, stores the solution in mwis_solution (0/1 per vertex).
/***************************************************************************/
static double MWIS_CPLEX(Graph *G, double *weights, int n, vector<int> &mwis_solution)
{
	IloEnv env;
	double obj_val = 0.0;

	try {
		IloModel model(env);

		// Binary variables x_i
		IloBoolVarArray x(env, n);

		// Objective: maximize sum w_i * x_i
		IloExpr obj(env);
		for (int i = 0; i < n; i++)
			obj += weights[i] * x[i];
		model.add(IloMaximize(env, obj));
		obj.end();

		// Constraints: x_i + x_j <= 1 for all edges (i,j)
		for (int e = 0; e < G->nedges; e++)
		{
			model.add(x[G->tail[e]] + x[G->head[e]] <= 1);
		}

		IloCplex cplex(model);
		cplex.setOut(env.getNullStream());
		cplex.setParam(IloCplex::Param::Threads, 1);
		cplex.solve();

		obj_val = cplex.getObjValue();

		mwis_solution.resize(n);
		for (int i = 0; i < n; i++)
			mwis_solution[i] = (cplex.getValue(x[i]) > 0.5) ? 1 : 0;

		cplex.end();
		model.end();

	} catch (IloException &e) {
		cerr << "CPLEX MWIS error: " << e.getMessage() << endl;
	}

	env.end();
	return obj_val;
}


/***************************************************************************/
// Check whether a set of vertices forms an independent set in graph G.
// Returns true if no two vertices in the set are adjacent.
/***************************************************************************/
static bool is_independent_set(Graph *G, const vector<int> &vertices)
{
	for (size_t i = 0; i < vertices.size(); i++)
		for (size_t j = i + 1; j < vertices.size(); j++)
			if (G->adj_matrix[vertices[i]][vertices[j]])
				return false;
	return true;
}


/***************************************************************************/
void LPBound_Solve(instance *inst)
/***************************************************************************/
{
 try {
	int n = inst->G->nnodes;
	int m = inst->G->nedges;
	int k = inst->num_colors;

	// =====================================================================
	// Build the complement graph (needed for MWIS via TSM = MWC on complement)
	// =====================================================================
	Graph *G_comp = build_complement_graph(inst->G);

	// Allocate TSM data arrays
	inst->VERTEX_WEIGHTS_int = new int[n];
	inst->MWCP_X = new double[n];

	// =====================================================================
	// DUAL formulation via column generation.
	//
	// The primal LP is:
	//   max  sum_{e} c_e * y_e
	//   s.t. y_e <= x_u,  y_e <= x_v        for all e=(u,v)
	//        sum_{u in I_h} x_u <= 1         for all independent sets I_h
	//        x >= 0,  y free
	//
	// Its dual (after substituting beta_e = c_e - alpha_e) reduces to:
	//   min  sum_h lambda_h
	//   s.t. sum_{h: u in I_h} lambda_h
	//          + sum_{e: head(e)=u} alpha_e
	//          - sum_{e: tail(e)=u} alpha_e  >= D_u     for all u
	//        0 <= alpha_e <= c_e                        for all e
	//        lambda_h >= 0                              for all h
	//
	// where D_u = sum_{e: head(e)=u} c_e.
	//
	// The dual has only n constraints (one per vertex)
	// plus simple variable bounds.  Adding a new independent set becomes
	// adding a COLUMN (new lambda_h variable).
	//
	// The primal x* values are recovered as the dual multipliers of the
	// vertex constraints.
	// =====================================================================

	// --- Precompute edge incidence and D_u ---
	vector<vector<int>> tail_edges(n), head_edges(n);
	for (int e = 0; e < m; e++)
	{
		tail_edges[inst->G->tail[e]].push_back(e);
		head_edges[inst->G->head[e]].push_back(e);
	}

	vector<double> D(n, 0.0);
	for (int u = 0; u < n; u++)
		for (int e : head_edges[u])
			D[u] += inst->G->edge_weights[e];

	// --- Build the dual model ---
	IloEnv env;
	IloModel model(env);

	// Objective: minimize sum_h lambda_h  (columns added below)
	IloObjective objective = IloMinimize(env);
	model.add(objective);

	// Variables: alpha_e  with  0 <= alpha_e <= c_e
	IloNumVarArray alpha(env);
	for (int e = 0; e < m; e++)
		alpha.add(IloNumVar(env, 0.0, inst->G->edge_weights[e], ILOFLOAT));

	// Vertex constraints (one per vertex, rhs = D_u)
	IloRangeArray vertex_con(env);
	for (int u = 0; u < n; u++)
	{
		IloExpr lhs(env);
		for (int e : head_edges[u])
			lhs += alpha[e];
		for (int e : tail_edges[u])
			lhs -= alpha[e];
		vertex_con.add(IloRange(env, D[u], lhs, IloInfinity));
		lhs.end();
	}
	model.add(vertex_con);

	// --- Initial lambda columns from the coloring heuristic ---
	vector<vector<int>> color_classes(k);
	for (int i = 0; i < n; i++)
		color_classes[inst->v_color[i]].push_back(i);

	IloNumVarArray lambda_vars(env);
	for (int h = 0; h < k; h++)
	{
		// Column: cost 1 in objective, coefficient +1 in vertex_con[u] for u in I_h
		IloNumColumn col = objective(1.0);
		for (int u : color_classes[h])
			col += vertex_con[u](1.0);
		lambda_vars.add(IloNumVar(col, 0.0, IloInfinity, ILOFLOAT));
		col.end();
	}

	// =====================================================================
	// Column-generation loop  (dual ↔ cutting planes on the primal)
	// =====================================================================

	IloCplex cplex(model);

#ifdef DISABLE_CPLEX_OUTPUT_LP
	cplex.setOut(env.getNullStream());
#endif

	cplex.setParam(IloCplex::Param::Threads, 1);

	// Barrier without crossover — fast on the dual (set once)
	cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_BARRIER);
	cplex.setParam(IloCplex::Param::SolutionType, 2);          // accept non-basic sol (no crossover)

	int num_cuts = 0;
	int max_iterations = 10000;
	bool time_limit_hit = false;

	// Wall-clock timer
	auto wall_start = chrono::steady_clock::now();

	// TSM reads from inst->G — we will temporarily swap it to the complement
	Graph *G_original = inst->G;

	for (int iter = 0; iter < max_iterations; iter++)
	{
		// --- Check remaining time before LP solve ---
		double elapsed = chrono::duration<double>(
			chrono::steady_clock::now() - wall_start).count();
		double remaining = inst->PARAM_TIME_LIMIT - elapsed;
		if (remaining <= 0.0)
		{
			cout << "LP Bound: time limit reached before LP solve (iter "
			     << iter << ")." << endl;
			time_limit_hit = true;
			break;
		}

		cplex.setParam(IloCplex::Param::TimeLimit, remaining);

		// Solve the dual LP
		try {
			cplex.solve();
		} catch (IloException &e) {
			cerr << "CPLEX solve exception: " << e.getMessage() << endl;
			time_limit_hit = true;
			break;
		}

		auto status = cplex.getStatus();
		if (status != IloAlgorithm::Optimal && status != IloAlgorithm::Feasible)
		{
			cout << "Dual LP solve status: " << status << " — stopping." << endl;
			time_limit_hit = true;
			break;
		}

		double dual_obj = cplex.getObjValue();

		// --- Recover primal x* from dual multipliers of vertex constraints ---
		IloNumArray duals(env);
		try {
			cplex.getDuals(duals, vertex_con);
		} catch (IloException &e) {
			cerr << "CPLEX getDuals exception: " << e.getMessage() << endl;
			time_limit_hit = true;
			break;
		}

		vector<double> x_star(n);
		for (int u = 0; u < n; u++)
			x_star[u] = (duals[u] > 0.0) ? duals[u] : 0.0;   // clamp numerical noise
		duals.end();

		// --- Check remaining time before separation ---
		elapsed = chrono::duration<double>(
			chrono::steady_clock::now() - wall_start).count();
		if (elapsed >= inst->PARAM_TIME_LIMIT)
		{
			cout << "LP Bound: time limit reached before separation (iter "
			     << iter << ")." << endl;
			time_limit_hit = true;
			break;
		}

		// =====================================================================
		// Separation: solve MWIS with weights x* on the original graph
		// (= MWC with weights x* on the complement graph via TSM)
		// =====================================================================

		if(iter == 0)
		{
			cout << "LP Bound: initial dual obj = " << dual_obj << "\n\n";
		}

		// Scale x* to integer weights for TSM
		for (int i = 0; i < n; i++)
		{
			inst->VERTEX_WEIGHTS_int[i] = (int) round(x_star[i] * SEPARATION_SCALE);
			inst->MWCP_X[i] = 0.0;
		}

		// Point inst->G to the complement graph for TSM
		inst->G = G_comp;

		// Set TSM time limit to remaining global time
		double tsm_remaining = inst->PARAM_TIME_LIMIT
			- chrono::duration<double>(chrono::steady_clock::now() - wall_start).count();
		set_TSM_time_limit(max(tsm_remaining, 0.0));

		// Run TSM (Maximum Weight Clique on complement = MWIS on original)
		int tsm_result = TSM_MWC(inst);
		(void) tsm_result;

		// Restore original graph
		inst->G = G_original;

		// Collect the independent set found by TSM
		vector<int> independent_set;
		double exact_weight = 0.0;
		for (int i = 0; i < n; i++)
		{
			if (inst->MWCP_X[i] > 0.5)
			{
				independent_set.push_back(i);
				exact_weight += x_star[i];
			}
		}

		// Free TSM internal structures for next call
		free_TSM();

		// If TSM hit the time limit, the MWIS solution may be suboptimal.
		// We cannot trust the separation result, so stop.
		if (TSM_TIME_LIMIT_HIT)
		{
			cout << "LP Bound: TSM time limit reached during separation (iter "
			     << iter << ")." << endl;
			time_limit_hit = true;
			break;
		}

#ifdef VERIFY_MWIS_WITH_CPLEX
		// Compare TSM's MWIS with CPLEX's exact MWIS (same integer-scaled weights)
		{
			vector<double> int_weights_dbl(n);
			for (int i = 0; i < n; i++)
				int_weights_dbl[i] = (double) inst->VERTEX_WEIGHTS_int[i];

			vector<int> cplex_sol;
			double cplex_obj = MWIS_CPLEX(G_original, int_weights_dbl.data(), n, cplex_sol);

			double tsm_int_obj = 0.0;
			for (int i : independent_set)
				tsm_int_obj += inst->VERTEX_WEIGHTS_int[i];

			bool tsm_valid = !independent_set.empty()
				&& is_independent_set(G_original, independent_set);

			if (!tsm_valid || tsm_int_obj < cplex_obj - 0.5)
			{
				cout << "  [VERIFY] iter " << iter
					 << ": TSM=" << (int)tsm_int_obj
					 << (tsm_valid ? " (valid)" : " (INVALID)")
					 << " CPLEX=" << (int)cplex_obj
					 << " — using CPLEX solution" << endl;

				independent_set.clear();
				exact_weight = 0.0;
				for (int i = 0; i < n; i++)
				{
					if (cplex_sol[i])
					{
						independent_set.push_back(i);
						exact_weight += x_star[i];
					}
				}
			}
			else if (iter % 50 == 0)
			{
				cout << "  [VERIFY] iter " << iter
					 << ": TSM=" << (int)tsm_int_obj
					 << " CPLEX=" << (int)cplex_obj
					 << " — OK" << endl;
			}
		}
#endif

		// Verify that TSM's solution is actually an independent set
		if (!independent_set.empty() && !is_independent_set(G_original, independent_set))
		{
			cout << "  TSM returned invalid IS (iter " << iter
			     << "), falling back to CPLEX MWIS." << endl;

			vector<int> cplex_mwis_sol;
			MWIS_CPLEX(G_original, x_star.data(), n, cplex_mwis_sol);

			independent_set.clear();
			exact_weight = 0.0;
			for (int i = 0; i < n; i++)
			{
				if (cplex_mwis_sol[i])
				{
					independent_set.push_back(i);
					exact_weight += x_star[i];
				}
			}
		}

		// --- Check remaining time after separation ---
		elapsed = chrono::duration<double>(
			chrono::steady_clock::now() - wall_start).count();
		if (elapsed >= inst->PARAM_TIME_LIMIT)
		{
			cout << "LP Bound: time limit reached after separation (iter "
			     << iter << ")." << endl;
			time_limit_hit = true;
			break;
		}

		// --- Pricing check ---
		if (exact_weight <= 1.0 + 1e-6)
		{
			// No violated IS ⇒ dual is optimal ⇒ LP bound converged
			cout << "LP Bound converged after " << iter + 1
			     << " iterations (" << num_cuts << " cols added)." << endl;
			cout << "LP Bound value: " << dual_obj << endl;
			break;
		}

		// --- Add new column to the dual model ---
		IloNumColumn col = objective(1.0);
		for (int u : independent_set)
			col += vertex_con[u](1.0);
		lambda_vars.add(IloNumVar(col, 0.0, IloInfinity, ILOFLOAT));
		col.end();

		num_cuts++;

		if (iter % 10 == 0)
		{
			cout << fixed << setprecision(1) 
				 << chrono::duration<double>(chrono::steady_clock::now() - wall_start).count() << "s --> Iter " << iter 
				 << ": Dual obj = " << fixed << setprecision(1) << dual_obj
				 << ", MWIS weight = " << fixed << setprecision(1) << exact_weight
				 << ", IS size = " << independent_set.size()
				 << ", cols = " << num_cuts << endl;
		}
	}

	// Restore original graph
	inst->G = G_original;

	// Store final result
	try {
		inst->LPBound = floor(cplex.getObjValue());
	} catch (...) {
		inst->LPBound = -1.0;
	}
	inst->LPBound_num_cuts = num_cuts;
	inst->LPBound_TimeLimitHit = time_limit_hit;

	// Cleanup
	cplex.end();
	model.end();
	env.end();

	delete G_comp;
	delete[] inst->VERTEX_WEIGHTS_int;
	inst->VERTEX_WEIGHTS_int = nullptr;
	delete[] inst->MWCP_X;
	inst->MWCP_X = nullptr;

 } catch (IloException &e) {
	cerr << "CPLEX Exception in LPBound_Solve: " << e.getMessage() << endl;
	inst->LPBound = -1.0;
	inst->LPBound_num_cuts = 0;
	inst->LPBound_TimeLimitHit = true;
 }
}
