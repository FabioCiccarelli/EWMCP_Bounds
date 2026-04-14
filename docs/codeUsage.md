# Code Usage

## Build

The project is built with the root [Makefile](../Makefile) and expects the CPLEX installation path in `CPLEX_PATH`.

```bash
export CPLEX_PATH=/path/to/CPLEX_Studio
make
```

The executable is written to `./bin/EWMCP_BOUNDS`.

## Command Line

Standard mode:

```bash
./bin/EWMCP_BOUNDS <instance_path> --bound <SH|SS|SSpooled|HFB|CG|F1|F11> [options]
```

Branching-study mode:

```bash
./bin/EWMCP_BOUNDS <instance_path> --bound <SH|SS|HFB> --test-branching --incumbent <value> [options]
```

The weights file is always derived automatically as `<instance_path>.weights`.

## Parameters

| Parameter | Meaning | Values | Default |
| --- | --- | --- | --- |
| `instance_path` | Input graph in DIMACS format | path to `.clq` file | - |
| `--bound` | Bounding approach | `SH`, `SS`, `SSpooled`, `HFB`, `CG`, `F1`, `F11` | required |
| `--coloring` | Coloring method | `dsatur`, `random` | `dsatur` |
| `--seed` | Random seed | integer | `-1` |
| `--time-limit` | Time limit in seconds | positive number | `3600` |
| `--sorting-strategy` | Stable-set ordering for `SH` | `natural`, `size`, `weight` | `natural` |
| `--sorting-sense` | Ordering direction for `SH` | `1`, `-1` | `1` |
| `--disable-valid-ineq` | Drop valid inequalities in `F11` | flag | off |
| `--test-branching` | Enable single-branch branching study | flag | off |
| `--incumbent` | Incumbent value used for pruning in branching mode | positive real | required with `--test-branching` |

## Supported Bounds

- `SS`: San Segundo et al. LP bound.
- `SSpooled`: San Segundo bound with a pooled family of independent sets obtained from one DSATUR coloring plus five random colorings.
- `SH`: Shimizu et al. bound.
- `HFB`: Hosseinian et al. bound.
- `CG`: column-generation LP bound with independent-set separation.
- `F11`: compact LP formulation with valid inequalities.
- `F1`: shorthand for `F11 --disable-valid-ineq`.

Notes:

- `SSpooled` handles its own set construction internally; `--coloring` and `--seed` do not change its behavior.
- `--sorting-strategy` and `--sorting-sense` are only meaningful for `SH`.
- In output files, `F1` is reported explicitly as `F1` even though the solver path is implemented through `F11` with valid inequalities disabled.

## Input Format

The graph file must be in DIMACS edge format:

```text
p edge n m
e i j
e i j
...
```

The companion weights file must contain one edge weight per line, in the same order as the edges in the graph file:

```text
w1
w2
...
wm
```

## Examples

```bash
# Shimizu bound with DSATUR coloring
./bin/EWMCP_BOUNDS path/to/instance.clq --bound SH --coloring dsatur

# Shimizu bound with size-descending stable-set order
./bin/EWMCP_BOUNDS path/to/instance.clq --bound SH --coloring dsatur --sorting-strategy size --sorting-sense -1

# San Segundo pooled bound
./bin/EWMCP_BOUNDS path/to/instance.clq --bound SSpooled --time-limit 3600

# Column-generation bound
./bin/EWMCP_BOUNDS path/to/instance.clq --bound CG --coloring dsatur --time-limit 3600

# F11 without valid inequalities, reported as F1
./bin/EWMCP_BOUNDS path/to/instance.clq --bound F1 --coloring dsatur --time-limit 3600

# Single-branch branching study
./bin/EWMCP_BOUNDS path/to/instance.clq --bound SH --test-branching --incumbent 1000 --coloring dsatur --seed 7
```

## Standard Output File

In standard mode, the program appends one line to `results.txt` in the current working directory. The format is uniform across all bounds:

| Column | Meaning |
| --- | --- |
| 1 | instance path |
| 2 | approach label |
| 3 | coloring label |
| 4 | seed |
| 5 | time limit |
| 6 | sorting strategy |
| 7 | sorting sense |
| 8 | number of vertices |
| 9 | number of edges |
| 10 | number of colors / initial sets |
| 11 | primary bound value |
| 12 | secondary value |
| 13 | status |
| 14 | elapsed time |

The secondary field depends on the bound:

- `SH`: Policy 1 bound.
- `SS` and `SSpooled`: CPLEX best bound.
- `HFB`: `none`.
- `CG`: number of generated columns.
- `F11` and `F1`: `none`.

The status field is `Optimal` for `SH` and `HFB`, CPLEX-derived for `SS` and `SSpooled`, and either `Optimal` or `TimeLimit` for `CG`, `F11` and `F1`.

## Branching Output Files

In branching-study mode, the program writes to a bound-specific file in the current working directory:

- `branching_results_SH.txt`
- `branching_results_SS.txt`
- `branching_results_HFB.txt`

See [branchingStudy.md](branchingStudy.md) for the exact format and interpretation.

## References

- [../README.md](../README.md)
- [branchingStudy.md](branchingStudy.md)
