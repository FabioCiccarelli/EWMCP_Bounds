# 💻 Code Usage Guide

<div align="center">

[![C++](https://img.shields.io/badge/Language-C++-blue.svg)](https://www.cplusplus.com/)
[![CPLEX](https://img.shields.io/badge/Requires-CPLEX-orange.svg)](https://www.ibm.com/products/ilog-cplex-optimization-studio)

</div>

---

## 🚀 Quick Start

The executable generated from this code follows this command structure:

```bash
./EWMCP_BOUNDS <instance_path> --bound <SH|SS|HFB> [options]
```

The edge weights file is derived automatically as `<instance_path>.weights`.

## 📋 Parameters Reference

| Parameter | Description | Options | Default |
|-----------|-------------|---------|--------|
| `instance_path` | Path to the input graph file (positional, required) | DIMACS format file | — |
| `--bound` | Bounding approach to use (required) | `SS`, `SH`, or `HFB` | — |
| `--coloring` | Graph coloring method | `dsatur` or `random` | `dsatur` |
| `--seed` | Random seed for coloring | Integer | `-1` |
| `--time-limit` | Maximum runtime in seconds | Positive number | `3600` |
| `--sorting-strategy` | Stable set sorting strategy (only for `SH`) | `natural`, `size`, or `weight` | `natural` |
| `--sorting-sense` | Sorting direction (only for `size` and `weight` strategies) | `1` (ascending) or `-1` (descending) | `1` |

### 🔵 Approach Options

- **`SS`** - San Segundo et al. bound ([EJOR 2019](https://doi.org/10.1016/j.ejor.2019.03.047))
- **`SH`** - Shimizu et al. bound ([Discrete Optimization 2020](https://doi.org/10.1016/j.disopt.2020.100583))
- **`HFB`** - Hosseinian et al. bound ([IJOC 2020](https://doi.org/10.1287/ijoc.2019.0898))

### 🎨 Coloring Method Options

- **`dsatur`** - DSATUR algorithm (recommended)
- **`random`** - Random coloring method

### 🔄 Sorting Strategy Options (Shimizu bound only)

The Shimizu bound depends on the ordering of the stable sets (color classes). Three strategies are available:

- **`natural`** — Uses the ordering returned by the coloring algorithm without any modification. This is the default.
- **`size`** — Sorts the stable sets by the number of vertices they contain.
- **`weight`** — Sorts the stable sets by a score computed as follows. Given *k* stable sets, for each vertex *v* a score γ(*v*) is computed as the sum of the weights of the *k*-1 heaviest edges incident on *v*. The score of a stable set is then the sum of γ(*v*) over all its vertices.

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

## 📤 Output Format

The program writes results to **`results.txt`** (appending). Each line contains the same columns regardless of the approach, separated by tabs:

| Column | Description |
|--------|-------------|
| Instance | Path to input graph |
| Approach | `SS`, `SH`, or `HFB` |
| Coloring | `dsatur` or `random` |
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
| **HFB** | HFB bound value | `none` | `Optimal` |

---

<div align="center">
<sub>For more information, see the main <a href="README.md">README</a> or the <a href="additional_results.md">additional results</a></sub>
</div>
