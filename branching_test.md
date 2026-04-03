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
| `--sorting-strategy <natural\|size\|weight>` | Stable set sorting strategy (SH only, see [code_usage.md](code_usage.md)) | `natural` |
| `--sorting-sense <1\|-1>` | Sorting direction: 1=ascending, -1=descending (SH only) | `1` |

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
| Per-depth snapshots | One tab-separated token per depth, each formatted as `W_C\|L\|bound\|best_bound\|trivial` |

The per-depth tokens encode the state at that depth:
- `W_C` is the weight of the current clique C
- `|L|` is the number of remaining candidates
- `bound` is the **total** upper bound value `UB(C, L) = W(C) + UB_L` computed at that depth
- `best_bound` is the minimum **total** bound seen up to that depth (monotonically decreasing)
- `trivial` is a **total** trivial upper bound = W(C) + sum_gamma + sum_internal

**Important:** `bound`, `best_bound`, and `trivial` all include `W_C` — they are total bounds, not just the `G[L]` contribution. The pruning check uses `best_bound` to ensure monotonic pruning behavior.

**Example** (3 depths, tab-separated after the fixed columns):
```
... \t 0|155|137157|137157|500000 \t 36|126|110936|110936|450000 \t 247|98|110000|110000|400000
```

In this example:
- At depth 0: W(C)=0, |L|=155, total bound=137157
- At depth 1: W(C)=36, |L|=126, total bound=110936 (decreased)
- At depth 2: W(C)=247, |L|=98, raw bound=110000 (slight decrease), best_bound stays at 110000

### Console output

During execution, each depth is printed on a single line including the index of the vertex fixed at that step (`v=<index>`), `|C|`, `|L|`, `W(C)`, and the bound value. A summary table with the full trace is printed at the end.

### Interpreting the output

- **Depth**: The number of vertices in the clique `C` when the simulation stopped. Lower depth with `PRUNED_BY_BOUND` indicates a stronger bound.
- **Stop reason**: `PRUNED_BY_BOUND` means the bound was tight enough to prune; `CANDIDATES_EMPTY` means the random branch reached a maximal clique without pruning.
- **Trace**: The evolution of `(W_C, |L|, bound, best_bound)` as vertices are added to `C`. The `best_bound` sequence is monotonically non-increasing by construction. A quickly decreasing `best_bound` indicates the bound tightens effectively with depth.

## Implementation details

### Coloring at each depth

At each step, the subgraph `G[L]` is re-colored from scratch using either DSATUR or random greedy coloring. While incremental coloring updates would be more efficient, they would compromise coloring quality. For the sake of this analysis, the re-coloring cost is acceptable.

### Why bounds can increase with depth

Because `G[L]` is **recolored from scratch** at each depth, the coloring-based bounds (SH, HFB) can produce a different partition into independent sets even as `|L|` decreases. Combined with the fact that `W(C)` increases as vertices are added to the clique, the total bound `UB(C,L) = W(C) + UB_L` can occasionally **increase** from one depth to the next.

This is **not a bug** — both bounds are valid upper bounds. However, for monotonic pruning behavior, we track `best_bound = min(UB)` across all depths and use this value for the pruning check. The raw `bound` value is still recorded for analysis purposes.

### Gamma computation

The vector `gamma(v) = Σ_{u ∈ C} w(u,v)` is recomputed at each depth. This captures the contribution of candidate vertices toward the partial clique and is used by all three bounds.

### Source files

- [`src/BranchingTest.h`](src/BranchingTest.h) — Header with `BranchingTestResult` struct and function declarations
- [`src/BranchingTest.cpp`](src/BranchingTest.cpp) — Full implementation of the simulation and bound computations on `(C, L)`
