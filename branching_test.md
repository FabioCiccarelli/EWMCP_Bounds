# Single-Branch B&B Simulation Test

## Overview

This module implements a **single-branch Branch-and-Bound simulation** to evaluate the pruning strength of the SS, SH, and HFB upper bounds in a realistic B&B context. Since a full Branch-and-Bound implementation is beyond the scope of this project, we simulate one random path through the search tree and measure how quickly (at what depth) each bound is able to prune.

## Algorithm

1. **Initialization**: Start with an empty clique `C = {}` and candidate list `L = V` (all vertices).

2. **Vertex selection**: Pick a random vertex `v` from `L` (using the provided random seed) and add it to `C`.

3. **Update candidates**: Set `L = L ∩ N(v)` — keep only vertices adjacent to all vertices in `C`.

4. **Bound computation**: If `L` is non-empty, compute the upper bound `UB(C, L)` for the chosen approach:
   - **SH**: Color `G[L]`, compute `sigma(v) = gamma(v) + internal_SH(v)`, sum max per color class.
   - **HFB**: Color `G[L]` to get `omega_L`, compute `Gamma_bar(v) = gamma(v) + 0.5 * sum_top_(omega_L-1)`, sum top `omega_L` values.
   - **SS**: Color `G[L]`, build and solve the San Segundo LP on `G[L]` with gamma offsets.

   In all cases, `gamma(v) = Σ_{u ∈ C} w(u,v)` captures the contribution of each candidate vertex toward the current clique.

5. **Pruning check**: If `UB(C, L) ≤ incumbent`, stop (pruned by bound). Otherwise, go to step 2.

6. **Termination**: The search also stops if `L` becomes empty (no more candidates to extend the clique).

## Usage

```bash
./EWMCP_BOUNDS <instance_path> --bound <SH|SS|HFB> --test-branching --incumbent <value> [options]
```

### Required parameters

| Parameter | Description |
|-----------|-------------|
| `--bound <SH\|SS\|HFB>` | The upper bound to test (only SH, SS, HFB are supported) |
| `--test-branching` | Enable the single-branch B&B simulation mode |
| `--incumbent <value>` | The incumbent (lower bound) value used for pruning |

### Optional parameters

| Parameter | Description | Default |
|-----------|-------------|---------|
| `--coloring <dsatur\|random>` | Coloring method used on `G[L]` at each depth | `dsatur` |
| `--seed <int>` | Random seed for both vertex selection and coloring | `-1` |
| `--time-limit <seconds>` | Time limit for LP solves (SS bound) | `3600` |
| `--sorting-strategy` | Not applicable in branching test mode | — |

### Examples

```bash
# SH bound, dsatur coloring, incumbent = 21230
./EWMCP_BOUNDS data/DIMACS/brock200_1.clq --bound SH --test-branching --incumbent 21230 --coloring dsatur --seed 42

# HFB bound, random coloring, seed 123
./EWMCP_BOUNDS data/DIMACS/brock200_1.clq --bound HFB --test-branching --incumbent 21230 --coloring random --seed 123

```

## Output

Results are appended to **`branching_results.txt`** (one line per run). The columns are tab-separated:

| Column | Description |
|--------|-------------|
| Instance | Path to the input graph file |
| Approach | `SH`, `SS`, or `HFB` |
| Coloring | `dsatur` or `random` |
| Seed | Random seed used |
| Vertices | Number of graph vertices |
| Edges | Number of graph edges |
| Depth | Depth at which the simulation stopped |
| Stop reason | `PRUNED_BY_BOUND` or `CANDIDATES_EMPTY` |
| Incumbent | The incumbent value provided by the user |
| Time | Total computation time (seconds) |
| Per-depth snapshots | One tab-separated token per depth, each formatted as `W_C\|\|L\||bound` |

The per-depth tokens encode the state at that depth: `W_C` is the weight of the current clique, `|L|` is the number of remaining candidates, and `bound` is the upper bound value `UB(C, L)`.

**Example** (3 depths, tab-separated after the fixed columns):
```
... \t 0|155|137157 \t 36|126|110936 \t 247|98|76999
```

### Console output

During execution, each depth is printed on a single line including the index of the vertex fixed at that step (`v=<index>`), `|C|`, `|L|`, `W(C)`, and the bound value. A summary table with the full trace is printed at the end.

### Interpreting the output

- **Depth**: The number of vertices in the clique `C` when the simulation stopped. Lower depth with `PRUNED_BY_BOUND` indicates a stronger bound.
- **Stop reason**: `PRUNED_BY_BOUND` means the bound was tight enough to prune; `CANDIDATES_EMPTY` means the random branch reached a maximal clique without pruning.
- **Trace**: The evolution of `(W_C, |L|, bound)` as vertices are added to `C`. A quickly decreasing bound sequence indicates the bound tightens effectively with depth.

## Implementation details

### Coloring at each depth

At each step, the subgraph `G[L]` is re-colored from scratch using either DSATUR or random greedy coloring. While incremental coloring updates would be more efficient, they would compromise coloring quality. For the sake of this analysis, the re-coloring cost is acceptable.

### Gamma computation

The vector `gamma(v) = Σ_{u ∈ C} w(u,v)` is recomputed at each depth. This captures the contribution of candidate vertices toward the partial clique and is used by all three bounds.

### Source files

- [`src/BranchingTest.h`](src/BranchingTest.h) — Header with `BranchingTestResult` struct and function declarations
- [`src/BranchingTest.cpp`](src/BranchingTest.cpp) — Full implementation of the simulation and bound computations on `(C, L)`
