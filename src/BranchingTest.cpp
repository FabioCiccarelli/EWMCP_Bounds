#include "BranchingTest.h"
#include "SanSegundoBound.h"
#include "ShimizuBound.h"
#include "HFBBound.h"

///////////////////////////////////////////////////////////////////////////////
// Internal helpers — file-scope only
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief Color the subgraph induced by 'vertices' of inst->G.
 *
 * Allocates and fills color_out[0..n-1] where only vertices in the set get
 * a valid color; others are set to -1.  Returns number of colors used.
 *
 * @param inst          instance (graph + params)
 * @param vertices      sorted vector of vertex indices forming L
 * @param color_out     output: allocated array of size inst->G->nnodes
 * @return              number of colors used (omega_L)
 */
static int color_subgraph(instance *inst, const vector<int> &vertices, int *&color_out)
{
    int n = inst->G->nnodes;
    color_out = new int[n];
    for (int i = 0; i < n; i++) color_out[i] = -1;

    // Build a set for fast membership checks
    vector<bool> in_L(n, false);
    for (int v : vertices) in_L[v] = true;

    if (inst->PARAM_COLORING_METHOD == "dsatur")
    {
        // Build subgraph adjacency for dsatur library (string-based)
        map<string, vector<string>> subgraph;
        for (int v : vertices)
        {
            string sv = to_string(v);
            vector<string> nbrs;
            for (int k = 0; k < inst->G->node_degree[v]; k++)
            {
                int u = inst->G->adj_lists[v][k];
                if (in_L[u]) nbrs.push_back(to_string(u));
            }
            subgraph[sv] = nbrs;
        }

        Dsatur *alg = new Dsatur(subgraph);
        alg->color();
        int nc = alg->get_num_colors();
        for (int v : vertices)
            color_out[v] = alg->get_color(to_string(v));
        delete alg;
        return nc;
    }
    else // "random"
    {
        // Greedy random coloring on subgraph induced by vertices
        mt19937 rng(inst->PARAM_RANDOM_SEED);

        vector<int> order(vertices);
        shuffle(order.begin(), order.end(), rng);

        int num_colors = 0;
        for (int v : order)
        {
            // Find used colors among v's neighbors that are in L
            vector<bool> used(vertices.size() + 1, false);
            for (int k = 0; k < inst->G->node_degree[v]; k++)
            {
                int u = inst->G->adj_lists[v][k];
                if (in_L[u] && color_out[u] >= 0)
                    used[color_out[u]] = true;
            }
            int c = 0;
            while (c < (int)used.size() && used[c]) c++;
            color_out[v] = c;
            if (c + 1 > num_colors) num_colors = c + 1;
        }
        return num_colors;
    }
}

/**
 * @brief Compute gamma(v) = sum of w(u,v) for u in C, for every v in L.
 */
static void compute_gamma(instance *inst,
                           const vector<int> &C,
                           const vector<int> &L,
                           vector<double> &gamma)
{
    int n = inst->G->nnodes;
    gamma.assign(n, 0.0);

    // Build a set for C membership
    vector<bool> in_C(n, false);
    for (int u : C) in_C[u] = true;

    for (int v : L)
    {
        double g = 0.0;
        for (int k = 0; k < inst->G->node_degree[v]; k++)
        {
            int u = inst->G->adj_lists[v][k];
            if (in_C[u])
            {
                int e = inst->G->adj_edge_idx[v][k];
                g += inst->G->edge_weights[e];
            }
        }
        gamma[v] = g;
    }
}

/**
 * @brief Compute W(C) = total weight of edges inside the clique C.
 */
static double compute_clique_weight(instance *inst, const vector<int> &C)
{
    double w = 0.0;
    vector<bool> in_C(inst->G->nnodes, false);
    for (int u : C) in_C[u] = true;

    for (int e = 0; e < inst->G->nedges; e++)
    {
        if (in_C[inst->G->tail[e]] && in_C[inst->G->head[e]])
            w += inst->G->edge_weights[e];
    }
    return w;
}

/**
 * @brief Trivial upper bound on partial solution (C, L).
 *
 * Sums all edge weights that could appear in any clique F with C \subseteq F \subseteq C \cup L:
 *   - W(C)              : edges already in the clique
 *   - sum_{v in L} gamma(v)  : edges between C and L (each counted once from L's side)
 *   - sum_{e in E[L]} w(e)   : all edges inside L
 *
 * This is a valid but loose upper bound.
 */
static double bound_trivial(instance *inst,
                             const vector<int> &L,
                             const vector<double> &gamma,
                             double W_C)
{
    int n = inst->G->nnodes;
    vector<bool> in_L(n, false);
    for (int v : L) in_L[v] = true;

    // sum of gamma values
    double sum_gamma = 0.0;
    for (int v : L) sum_gamma += gamma[v];

    // sum of edge weights inside G[L] (each edge counted once)
    double sum_internal = 0.0;
    for (int e = 0; e < inst->G->nedges; e++)
    {
        if (in_L[inst->G->tail[e]] && in_L[inst->G->head[e]])
            sum_internal += inst->G->edge_weights[e];
    }

    return floor(W_C + sum_gamma + sum_internal);
}

///////////////////////////////////////////////////////////////////////////////
// Bound computations on (C, L) — return UB(C, L)
///////////////////////////////////////////////////////////////////////////////

/**
 * @brief SH bound on partial solution (C, L).
 *
 * UB(C,L) = W(C) + sum_h max_{v in I_h} sigma(v)
 * where sigma(v) = gamma(v) + internal_SH(v)
 */
static double bound_SH(instance *inst,
                        const vector<int> &C,
                        const vector<int> &L,
                        const vector<double> &gamma,
                        int *color_L,
                        int num_colors_L,
                        double W_C)
{
    int n = inst->G->nnodes;
    vector<bool> in_L(n, false);
    for (int v : L) in_L[v] = true;

    // --- Compute internal SH weights (Policy 2: MIW-based) on G[L] ---
    // MIW[v][h] = max edge weight from v to any vertex of color h in G[L], for h > color(v)
    // Then internal(v) = sum over h >= color(v) of MIW[v][h]

    // Allocate MIW only for vertices in L (use full-size arrays keyed by vertex id)
    // MIW[v][h] for v in L, h in [0, num_colors_L)
    vector<vector<double>> MIW(n, vector<double>(num_colors_L, 0.0));

    for (int v : L)
    {
        for (int k = 0; k < inst->G->node_degree[v]; k++)
        {
            int u = inst->G->adj_lists[v][k];
            if (!in_L[u]) continue;

            int h_v = color_L[v];
            int h_u = color_L[u];
            int e = inst->G->adj_edge_idx[v][k];
            double w_e = inst->G->edge_weights[e];

            if (h_v < h_u)
            {
                if (w_e > MIW[v][h_u]) MIW[v][h_u] = w_e;
            }
            else if (h_v > h_u)
            {
                // This edge is attributed to u, not v — handled when iterating over u
            }
        }
    }

    // sigma(v) = gamma(v) + sum_{h >= color(v)} MIW[v][h]
    vector<double> sigma(n, 0.0);
    for (int v : L)
    {
        double internal_v = 0.0;
        for (int h = color_L[v]; h < num_colors_L; h++)
            internal_v += MIW[v][h];
        sigma[v] = gamma[v] + internal_v;
    }

    // UB_L = sum over each color class h of max sigma(v) for v in that class
    double UB_L = 0.0;
    for (int h = 0; h < num_colors_L; h++)
    {
        double best = 0.0;
        for (int v : L)
        {
            if (color_L[v] == h && sigma[v] > best)
                best = sigma[v];
        }
        UB_L += best;
    }

    return floor(W_C + UB_L);
}

/**
 * @brief HFB bound on partial solution (C, L).
 *
 * UB(C,L) = W(C) + sum of top omega_L values of Gamma_bar(v)
 * where Gamma_bar(v) = gamma(v) + 0.5 * sum of top (omega_L - 1) edge weights in G[L]
 */
static double bound_HFB(instance *inst,
                         const vector<int> &C,
                         const vector<int> &L,
                         const vector<double> &gamma,
                         int omega_L,
                         double W_C)
{
    int n = inst->G->nnodes;
    vector<bool> in_L(n, false);
    for (int v : L) in_L[v] = true;

    vector<double> Gamma_bar(n, 0.0);

    for (int v : L)
    {
        // Collect edge weights incident to v within G[L]
        vector<double> weights;
        for (int k = 0; k < inst->G->node_degree[v]; k++)
        {
            int u = inst->G->adj_lists[v][k];
            if (in_L[u])
            {
                int e = inst->G->adj_edge_idx[v][k];
                weights.push_back(inst->G->edge_weights[e]);
            }
        }

        sort(weights.begin(), weights.end(), greater<double>());

        int top = min((int)weights.size(), omega_L - 1);
        double sum = 0.0;
        for (int j = 0; j < top; j++) sum += weights[j];

        Gamma_bar[v] = gamma[v] + 0.5 * sum;
    }

    // Sort L by Gamma_bar descending and sum top omega_L
    vector<int> sorted_L(L);
    sort(sorted_L.begin(), sorted_L.end(), [&](int a, int b) {
        return Gamma_bar[a] > Gamma_bar[b];
    });

    double UB_L = 0.0;
    int count = min((int)sorted_L.size(), omega_L);
    for (int i = 0; i < count; i++)
        UB_L += Gamma_bar[sorted_L[i]];

    return floor(W_C + UB_L);
}

/**
 * @brief SS bound on partial solution (C, L).
 *
 * Builds and solves the San Segundo LP on G[L] with gamma offsets.
 * UB(C,L) = W(C) + ObjVal(LP)
 */
static double bound_SS(instance *inst,
                        const vector<int> &C,
                        const vector<int> &L,
                        const vector<double> &gamma,
                        int *color_L,
                        int num_colors_L,
                        double W_C)
{
    int n = inst->G->nnodes;
    vector<bool> in_L(n, false);
    for (int v : L) in_L[v] = true;

    // Collect edges in G[L]
    vector<int> subedges; // indices into inst->G edges
    for (int e = 0; e < inst->G->nedges; e++)
    {
        if (in_L[inst->G->tail[e]] && in_L[inst->G->head[e]])
            subedges.push_back(e);
    }

    IloEnv env;
    try {
        IloModel model(env);

        int nE_L = (int)subedges.size();

        // rho[e][0], rho[e][1] for each edge e in G[L]
        IloArray<IloNumVarArray> rho(env, nE_L);
        for (int i = 0; i < nE_L; i++)
            rho[i] = IloNumVarArray(env, 2, 0, IloInfinity, ILOFLOAT);

        // pi[h] for each color class h
        IloNumVarArray pi(env, num_colors_L, 0, IloInfinity, ILOFLOAT);

        // Objective: minimize sum pi[h]
        IloExpr obj(env);
        for (int h = 0; h < num_colors_L; h++)
            obj += pi[h];
        model.add(IloMinimize(env, obj));
        obj.end();

        // Constraint 1: rho[e][0] + rho[e][1] = w_e  for e in E[L]
        for (int i = 0; i < nE_L; i++)
        {
            int e = subedges[i];
            IloExpr expr(env);
            expr += rho[i][0] + rho[i][1];
            model.add(expr == inst->G->edge_weights[e]);
            expr.end();
        }

        // Build per-vertex lists of (subedge_index, endpoint_position)
        // For each v in L: which subedges are incident and which side is v?
        // We need: for vertex u, sum of rho[i][side] for sub-edges i incident to u
        vector<vector<pair<int,int>>> vertex_subedges(n);
        for (int i = 0; i < nE_L; i++)
        {
            int e = subedges[i];
            int t = inst->G->tail[e], h = inst->G->head[e];
            vertex_subedges[t].push_back({i, 0});
            vertex_subedges[h].push_back({i, 1});
        }

        // Constraint 2: pi[color(u)] >= sum_{e in delta_L(u)} rho[e][u_side] + gamma(u)
        for (int u : L)
        {
            IloExpr expr(env);
            for (auto &[idx, side] : vertex_subedges[u])
                expr += rho[idx][side];
            model.add(expr + gamma[u] <= pi[color_L[u]]);
            expr.end();
        }

        // Solve
        IloCplex cplex(model);
        cplex.setOut(env.getNullStream());
        cplex.setParam(IloCplex::Param::TimeLimit, inst->PARAM_TIME_LIMIT);
        cplex.setParam(IloCplex::Param::Threads, 1);
        cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_BARRIER);
        cplex.setParam(IloCplex::Param::SolutionType, 2);

        cplex.solve();

        double UB_L = cplex.getObjValue();

        // Cleanup
        for (int i = 0; i < nE_L; i++) rho[i].end();
        rho.end();
        pi.end();
        cplex.end();
        model.end();
        env.end();

        return floor(W_C + UB_L);
    }
    catch (IloException &ex) {
        cerr << "CPLEX error in bound_SS: " << ex.getMessage() << endl;
        env.end();
        return 1e18; // huge value, won't prune
    }
}

///////////////////////////////////////////////////////////////////////////////
// BRANCHING TEST
///////////////////////////////////////////////////////////////////////////////

BranchingTestResult run_branching_test(instance *inst, double incumbent)
{
    BranchingTestResult result;
    result.incumbent = incumbent;
    result.pruned_by_bound = false;
    result.depth = 0;

    clock_t t_start = clock();

    int n = inst->G->nnodes;
    mt19937 rng(inst->PARAM_RANDOM_SEED);

    // C = current clique (vertex indices)
    vector<int> C;

    // L = candidate vertices (common neighbors of all vertices in C)
    // Initially L = V
    vector<int> L(n);
    for (int i = 0; i < n; i++) L[i] = i;

    // W(C) = weight of edges in C
    double W_C = 0.0;

    // best_UB = minimum bound seen so far (ensures monotonic decrease)
    double best_UB = 1e99;

    // --- Depth-0 snapshot: bound on the full graph before any vertex is fixed ---
    {
        int *color_L = nullptr;
        int num_colors_L = color_subgraph(inst, L, color_L);

        vector<double> gam;
        compute_gamma(inst, C, L, gam); // C is empty, so gamma = 0 everywhere

        double UB = 0.0;
        if (inst->PARAM_APPROACH == "SH")
            UB = bound_SH(inst, C, L, gam, color_L, num_colors_L, W_C);
        else if (inst->PARAM_APPROACH == "HFB")
            UB = bound_HFB(inst, C, L, gam, num_colors_L, W_C);
        else if (inst->PARAM_APPROACH == "SS")
            UB = bound_SS(inst, C, L, gam, color_L, num_colors_L, W_C);

        delete[] color_L;

        double trivial = bound_trivial(inst, L, gam, W_C);
        best_UB = UB;

        DepthSnapshot snap;
        snap.W_C        = W_C;
        snap.L_size     = (int)L.size();
        snap.bound      = UB;
        snap.best_bound = best_UB;
        snap.trivial    = trivial;
        result.bound_trace.push_back(snap);

        cout << "  Depth 0  v=-  |C|=0  |L|=" << L.size()
             << "  W(C)=0  UB=" << UB
             << "  best=" << best_UB
             << "  trivial=" << trivial
             << "  incumbent=" << incumbent << endl;
    }

    bool pruned = false;

    while (!L.empty())
    {
        // Pick a random vertex from L
        uniform_int_distribution<int> dist(0, (int)L.size() - 1);
        int idx = dist(rng);
        int v_chosen = L[idx];

        // Add v_chosen to C
        C.push_back(v_chosen);

        // Update W(C): add edges from v_chosen to all previous vertices in C
        for (int i = 0; i < (int)C.size() - 1; i++)
        {
            int u = C[i];
            if (inst->G->hasEdge(u, v_chosen))
            {
                // Find edge index
                for (int k = 0; k < inst->G->node_degree[v_chosen]; k++)
                {
                    if (inst->G->adj_lists[v_chosen][k] == u)
                    {
                        int e = inst->G->adj_edge_idx[v_chosen][k];
                        W_C += inst->G->edge_weights[e];
                        break;
                    }
                }
            }
        }

        // Update L: keep only vertices in L that are adjacent to v_chosen (and not v_chosen itself)
        vector<int> new_L;
        for (int u : L)
        {
            if (u != v_chosen && inst->G->hasEdge(u, v_chosen))
                new_L.push_back(u);
        }
        L = new_L;

        // If L is empty, we finished the branch (no more candidates)
        if (L.empty())
        {
            // Record final depth, no bound to compute
            result.depth = (int)C.size();
            result.pruned_by_bound = false;
            break;
        }

        // Compute the upper bound on (C, L)
        // 1. Color G[L]
        int *color_L = nullptr;
        int num_colors_L = color_subgraph(inst, L, color_L);

        // 2. Compute gamma
        vector<double> gam;
        compute_gamma(inst, C, L, gam);

        // 3. Compute the bound
        double UB = 0.0;
        if (inst->PARAM_APPROACH == "SH")
        {
            UB = bound_SH(inst, C, L, gam, color_L, num_colors_L, W_C);
        }
        else if (inst->PARAM_APPROACH == "HFB")
        {
            UB = bound_HFB(inst, C, L, gam, num_colors_L, W_C);
        }
        else if (inst->PARAM_APPROACH == "SS")
        {
            UB = bound_SS(inst, C, L, gam, color_L, num_colors_L, W_C);
        }

        delete[] color_L;

        double trivial = bound_trivial(inst, L, gam, W_C);
        
        // Update best bound (monotonic minimum)
        if (UB < best_UB) best_UB = UB;

        DepthSnapshot snap;
        snap.W_C        = W_C;
        snap.L_size     = (int)L.size();
        snap.bound      = UB;
        snap.best_bound = best_UB;
        snap.trivial    = trivial;
        result.bound_trace.push_back(snap);

        cout << "  Depth " << C.size() << "  v=" << v_chosen
             << "  |C|=" << C.size()
             << "  |L|=" << L.size()
             << "  W(C)=" << W_C
             << "  UB=" << UB
             << "  best=" << best_UB
             << "  trivial=" << trivial
             << "  incumbent=" << incumbent << endl;

        // Check pruning using best bound
        if (best_UB <= incumbent)
        {
            result.depth = (int)C.size();
            result.pruned_by_bound = true;
            pruned = true;
            break;
        }
    }

    if (!pruned && !result.pruned_by_bound)
    {
        result.depth = (int)C.size();
    }

    clock_t t_end = clock();
    result.total_time = (double)(t_end - t_start) / (double)CLOCKS_PER_SEC;

    // Summary
    cout << "\n--- Branching Test Summary ---\n";
    cout << "Approach: " << inst->PARAM_APPROACH << endl;
    cout << "Depth: " << result.depth << endl;
    cout << "Pruned by bound: " << (result.pruned_by_bound ? "YES" : "NO (L empty)") << endl;
    cout << "Incumbent: " << result.incumbent << endl;
    cout << "Bound trace  (W_C | |L| | bound | best | trivial):" << endl;
    for (const auto &s : result.bound_trace)
        cout << "  (" << s.W_C << "|" << s.L_size << "|" << s.bound << "|" << s.best_bound << "|" << s.trivial << ")" << endl;
    cout << "Time: " << result.total_time << " s" << endl;
    cout << "-----------------------------\n";

    return result;
}

///////////////////////////////////////////////////////////////////////////////
// Output
///////////////////////////////////////////////////////////////////////////////

void write_branching_results(instance *inst, const BranchingTestResult &result)
{
    string fname = string("branching_results_") + inst->PARAM_APPROACH + ".txt";
    ofstream out(fname, ios::app);
    if (!out.is_open()) {
        cerr << "Unable to open output file: " << fname << endl;
        return;
    }

    // Format per line:
    // instance \t approach \t coloring \t seed \t nnodes \t nedges \t
    // depth \t pruned_flag \t incumbent \t time \t
    // [W_C \t |L| \t bound \t best_bound \t trivial] for each depth (5 cols per depth)

    out << inst->istname_graph << "\t"
        << inst->PARAM_APPROACH << "\t"
        << inst->PARAM_COLORING_METHOD << "\t"
        << inst->PARAM_RANDOM_SEED << "\t"
        << inst->G->nnodes << "\t"
        << inst->G->nedges << "\t"
        << result.depth << "\t"
        << (result.pruned_by_bound ? "PRUNED_BY_BOUND" : "CANDIDATES_EMPTY") << "\t"
        << result.incumbent << "\t"
        << result.total_time << "\t";

    for (int i = 0; i < (int)result.bound_trace.size(); i++)
    {
        const auto &s = result.bound_trace[i];
        if (i > 0) out << "\t";
        out << s.W_C << "|" << s.L_size << "|" << s.bound << "|" << s.best_bound << "|" << s.trivial;
    }

    out << "\n";
    out.close();
}
