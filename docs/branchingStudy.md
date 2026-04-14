# Branching Study

## Overview

The executable supports a single-branch Branch-and-Bound simulation used to compare pruning strength of `SH`, `SS` and `HFB` along one random root-to-leaf path.

This is not a full exact B&B implementation. The goal is to study how quickly each bound can certify pruning as the partial clique grows and the candidate set shrinks.

## Command

```bash
./bin/EWMCP_BOUNDS <instance_path> --bound <SH|SS|HFB> --test-branching --incumbent <value> [options]
```

Relevant options:

| Parameter | Meaning | Default |
| --- | --- | --- |
| `--coloring <dsatur|random>` | Coloring method used on the current candidate subgraph | `dsatur` |
| `--seed <int>` | Seed for vertex selection and, if applicable, random coloring | `-1` |
| `--time-limit <seconds>` | LP time limit used by `SS` | `3600` |
| `--sorting-strategy <natural|size|weight>` | Stable-set ordering for `SH` | `natural` |
| `--sorting-sense <1|-1>` | Ordering direction for `SH` | `1` |

For the general CLI and build instructions, see [code_usage.md](code_usage.md).

## Simulation Logic

At each depth:

1. Start from a current clique `C` and a candidate set `L`.
2. Pick one vertex from `L` using the provided seed and add it to `C`.
3. Replace `L` with `L ∩ N(v)`.
4. Recompute the bound on the induced subgraph `G[L]`.
5. Form the total bound `UB(C, L) = W(C) + UB_L`.
6. Prune when the best total bound seen so far is not larger than the incumbent.

The run stops either with `PRUNED_BY_BOUND` or when `L` becomes empty.

## Output Files

Each run appends one line to a bound-specific file in the current working directory:

- `branching_results_SH.txt`
- `branching_results_SS.txt`
- `branching_results_HFB.txt`

Fixed columns:

| Column | Meaning |
| --- | --- |
| 1 | instance path |
| 2 | bound |
| 3 | coloring |
| 4 | seed |
| 5 | number of vertices |
| 6 | number of edges |
| 7 | stopping depth |
| 8 | stop reason: `PRUNED_BY_BOUND` or `CANDIDATES_EMPTY` |
| 9 | incumbent |
| 10 | total running time |

After the fixed columns, one tab-separated token is written for each visited depth. Each token has the format:

```text
W_C|L_size|bound|best_bound|trivial
```

where:

- `W_C` is the current clique weight.
- `L_size` is the current candidate-set size.
- `bound` is the raw total bound at that depth.
- `best_bound` is the minimum total bound observed up to that depth.
- `trivial` is the total trivial upper bound recorded for comparison.

## Interpretation Notes

- Smaller pruning depth usually indicates a stronger bound on that branch.
- `best_bound` is the quantity used for pruning, not the raw `bound` alone.
- The raw total bound can increase from one depth to the next because the subgraph is recolored from scratch at each step.
- This non-monotonicity is expected; the monotone quantity is `best_bound`.

## Implementation Pointers

- [../src/BranchingTest.cpp](../src/BranchingTest.cpp)
- [../src/BranchingTest.h](../src/BranchingTest.h)
- [../src/Main.cpp](../src/Main.cpp)