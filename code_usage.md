# 💻 Code Usage Guide

<div align="center">

[![C++](https://img.shields.io/badge/Language-C++-blue.svg)](https://www.cplusplus.com/)
[![CPLEX](https://img.shields.io/badge/Requires-CPLEX-orange.svg)](https://www.ibm.com/products/ilog-cplex-optimization-studio)

</div>

---

## 🚀 Quick Start

The executable generated from this code follows this command structure:

```bash
./EWMCP_BOUNDS <instance_path> --bound <SH|SS|SSpooled|HFB|CG|F1|F11> [options]
```

The edge weights file is derived automatically as `<instance_path>.weights`.

## 📋 Parameters Reference

| Parameter | Description | Options | Default |
|-----------|-------------|---------|--------|
| `instance_path` | Path to the input graph file (positional, required) | DIMACS format file | — |
| `--bound` | Bounding approach to use (required) | `SS`, `SSpooled`, `SH`, `HFB`, `CG`, `F1`, or `F11` | — |
| `--coloring` | Graph coloring method (not used by `SSpooled`) | `dsatur` or `random` | `dsatur` |
| `--seed` | Random seed for coloring | Integer | `-1` |
| `--time-limit` | Maximum runtime in seconds | Positive number | `3600` |
| `--sorting-strategy` | Stable set sorting strategy (only for `SH`) | `natural`, `size`, or `weight` | `natural` |
| `--sorting-sense` | Sorting direction (only for `size` and `weight` strategies) | `1` (ascending) or `-1` (descending) | `1` |
| `--disable-valid-ineq` | Disable valid inequalities (4)+(5) in F11 formulation | *(flag)* | off |

### 🔵 Approach Options

- **`SS`** - San Segundo et al. bound ([EJOR 2019](https://doi.org/10.1016/j.ejor.2019.03.047))
- **`SSpooled`** - San Segundo bound with pooled independent sets (DSATUR + 5 random colorings)
- **`SH`** - Shimizu et al. bound ([Discrete Optimization 2020](https://doi.org/10.1016/j.disopt.2020.100583))
- **`HFB`** - Hosseinian et al. bound ([IJOC 2020](https://doi.org/10.1287/ijoc.2019.0898))
- **`CG`** - Column generation bound with independent set separation
- **`F11`** - Compact LP formulation F11 ([Gouveia & Martins, 2015](https://doi.org/10.1007/s13675-014-0028-1))
- **`F1`** - Base compact LP formulation F1, i.e. F11 without valid inequalities (4)+(5) ([Gouveia & Martins, 2015](https://doi.org/10.1007/s13675-014-0028-1))

### 🎨 Coloring Method Options

- **`dsatur`** - DSATUR algorithm (recommended)
- **`random`** - Random coloring method

### 🔄 Sorting Strategy Options (Shimizu bound only)

The Shimizu bound depends on the ordering of the stable sets (color classes). Three strategies are available:

- **`natural`** — Uses the ordering returned by the coloring algorithm without any modification. This is the default.
- **`size`** — Sorts the stable sets by the number of vertices they contain.
- **`weight`** — Sorts the stable sets by a score computed as follows. Given *k* stable sets, for each vertex *v* a score γ(*v*) is computed as the sum of the weights of the *k*-1 heaviest edges incident on *v*. The score of a stable set is then the average of γ(*v*) over all its vertices.

The **`--sorting-sense`** parameter controls the sort direction:
- **`1`** (ascending): lightest/smallest stable sets receive lower indices, heaviest/largest receive higher indices.
- **`-1`** (descending): heaviest/largest stable sets receive lower indices, lightest/smallest receive higher indices.

## 📄 Input File Formats

### DIMACS Graph Format

The code reads graph instances in the standard DIMACS format:

```
p edge n m         # Problem line: n vertices, m edges
e i j              # Edge between vertices i and j (m lines)

```

### Weights File Format

The weights file should contain one weight per line, matching the exact order of edges in the instance file:

```
weight_1           # Weight for first edge
weight_2           # Weight for second edge
...
```

The weights file is expected at `<instance_path>.weights` (automatically derived from the instance path).

## 🔧 Example Usage

### San Segundo Bound

```bash
./EWMCP_BOUNDS ./brock200_1.clq --bound SS --coloring dsatur --time-limit 3600
```

This command:
- Uses graph `brock200_1.clq`
- Automatically loads weights from `brock200_1.clq.weights`
- Applies San Segundo bound (`SS`)
- Uses DSATUR coloring

### San Segundo Pooled Bound

```bash
./EWMCP_BOUNDS ./brock200_1.clq --bound SSpooled --time-limit 3600
```

This command:
- Uses graph `brock200_1.clq`
- Automatically loads weights from `brock200_1.clq.weights`
- Applies the San Segundo Pooled bound (`SSpooled`)
- Internally computes DSATUR coloring + 5 random colorings (seeds 1-5), pools all unique independent sets, and solves the San Segundo LP on this enriched set of independent sets
- The `--coloring` and `--seed` options are ignored for this approach

### Shimizu Bound with Weight-Based Sorting

```bash
./EWMCP_BOUNDS ./brock200_1.clq --bound SH --coloring dsatur --sorting-strategy weight --sorting-sense 1
```

This reorders the stable sets by weight score in ascending order (heaviest stable sets at higher color indices) before computing the Shimizu bound.

### Shimizu Bound with Size-Based Sorting

```bash
./EWMCP_BOUNDS ./brock200_1.clq --bound SH --coloring random --seed 42 --sorting-strategy size --sorting-sense -1
```

This uses a random coloring with seed 42, then sorts stable sets by size in descending order.

### HFB Bound

```bash
./EWMCP_BOUNDS ./brock200_1.clq --bound HFB --coloring dsatur --time-limit 60
```

### CG Bound

```bash
./EWMCP_BOUNDS ./brock200_1.clq --bound CG --coloring dsatur --time-limit 3600
```

The CG bound uses a column generation approach. It starts from a dual LP formulation with columns (associated to independent sets) derived from the coloring heuristic, then iteratively generates new columns by solving a Maximum Weighted Independent Set problem (via the TSM-MWC solver on the complement graph). The process terminates when no more negative reduced cost columns are found.

### F11 Bound

```bash
./EWMCP_BOUNDS ./brock200_1.clq --bound F11 --coloring dsatur --time-limit 3600
```

The F11 bound solves the compact LP relaxation F11 from [Gouveia & Martins (2015)](https://doi.org/10.1007/s13675-014-0028-1). The formulation uses vertex variables $x_i$ and edge variables $y_{ij}$, with linearization, non-edge, induced degree, and global cardinality constraints. The parameter $k$ (upper bound on the clique number) is set to the number of colors from the initial coloring. Solved via CPLEX barrier without crossover.

### F1 Bound

```bash
./EWMCP_BOUNDS ./brock200_1.clq --bound F1 --coloring dsatur --time-limit 3600
```

The F1 bound is the base compact LP formulation from [Gouveia & Martins (2015)](https://doi.org/10.1007/s13675-014-0028-1), i.e. F11 without the induced degree constraints (4) and the global cardinality constraint (5). Equivalently, one can use `--bound F11 --disable-valid-ineq`.

## 📤 Output Format

The program writes results to **`results.txt`** (appending). Each line contains the same columns regardless of the approach, separated by tabs:

| Column | Description |
|--------|-------------|
| Instance | Path to input graph |
| Approach | `SS`, `SSpooled`, `SH`, `HFB`, `CG`, `F1`, or `F11` |
| Coloring | `dsatur`, `random`, or `pooled` (for `SSpooled`) |
| Seed | Random seed value |
| Time limit | Time limit in seconds |
| Sorting strategy | `natural`, `size`, `weight`, or `none` (if not `SH`) |
| Sorting sense | `1`, `-1`, or `none` (if not applicable) |
| Vertices | Number of graph vertices |
| Edges | Number of graph edges |
| Num. colors | Number of colors used |
| Bound value | Main bound value |
| Bound value 2 | Secondary bound value (or `none`) |
| Status | Solution status |
| Time | Computation time in seconds |

### Approach-Specific Fields

| Approach | Bound value | Bound value 2 | Status |
|----------|------------|---------------|--------|
| **SH** | Shimizu bound (Policy 2) | Shimizu bound (Policy 1) | `Optimal` |
| **SS** | CPLEX objective value | CPLEX best bound | CPLEX status |
| **SSpooled** | CPLEX objective value | CPLEX best bound | CPLEX status |
| **HFB** | HFB bound value | `none` | `Optimal` |
| **CG** | CG bound value | Number of columns added | `Optimal` |
| **F11** | F11 bound value | `none` | `Optimal` |
| **F1** | F1 bound value | `none` | `Optimal` |

---

<div align="center">
<sub>For more information, see the main <a href="README.md">README</a> or the <a href="additional_results.md">additional results</a></sub>
</div>
