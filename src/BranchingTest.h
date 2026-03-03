#ifndef BRANCHINGTEST_HEADER
#define BRANCHINGTEST_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <random>
#include <set>

#include "global_functions.h"
#include "global_variables.h"

using namespace std;

/**
 * @brief Per-depth snapshot recorded during the single-branch simulation.
 */
struct DepthSnapshot {
    double W_C;      ///< Weight of the current clique C
    int    L_size;   ///< Number of candidates |L|
    double bound;    ///< Main upper bound value (SH / HFB / SS)
    double trivial;  ///< Trivial upper bound: W(C) + Σ gamma(v) + Σ w(e) for e in E[L]
};

/**
 * @brief Result structure for the single-branch B&B simulation.
 */
struct BranchingTestResult {
    int depth;                          ///< Depth at which the search stopped
    bool pruned_by_bound;               ///< true = pruned by bound, false = stopped because L is empty
    double incumbent;                   ///< Incumbent value provided by the user
    vector<DepthSnapshot> bound_trace;  ///< Snapshot at each depth (after the first vertex is added)
    double total_time;                  ///< Total wall-clock time of the simulation
};

/**
 * @brief Run the single-branch B&B simulation test.
 *
 * Randomly selects vertices to add to a clique C, computing the upper bound
 * at each depth. Stops when the bound allows pruning (UB <= incumbent) or
 * when the candidate list L becomes empty.
 *
 * Works with bounds: SH, SS, HFB.
 *
 * @param inst       Pointer to the instance structure (graph must be loaded)
 * @param incumbent  The incumbent value for pruning
 * @return BranchingTestResult with depth, pruning flag, and bound trace
 */
BranchingTestResult run_branching_test(instance *inst, double incumbent);

/**
 * @brief Write branching test results to a file.
 *
 * Appends a single line to "branching_results.txt" with:
 *   instance \t approach \t coloring \t seed \t nnodes \t nedges \t
 *   depth \t pruned_flag \t incumbent \t bound_d0 \t bound_d1 \t ...
 *
 * @param inst   Pointer to the instance structure
 * @param result The result of the branching test
 */
void write_branching_results(instance *inst, const BranchingTestResult &result);

#endif
