# 💻 Code Usage Guide

<div align="center">

[![C++](https://img.shields.io/badge/Language-C++-blue.svg)](https://www.cplusplus.com/)
[![CPLEX](https://img.shields.io/badge/Requires-CPLEX-orange.svg)](https://www.ibm.com/products/ilog-cplex-optimization-studio)

</div>

---

## 🚀 Quick Start

The executable generated from this code follows this command structure:

```bash
./EWMCP_BOUNDS <graph_file> <weights_file> <approach> <coloring_method> <random_seed> <time_limit>
```

## 📋 Parameters Reference

| Parameter | Description | Options |
|-----------|-------------|---------|
| `graph_file` | Path to the input graph file | DIMACS format file |
| `weights_file` | Path to the edge weights file | One weight per line |
| `approach` | Bounding approach to use | `SS` (San Segundo et al.), `SH` (Shimizu et al.), or `HFB` (Hosseinian et al.) |
| `coloring_method` | Graph coloring method | `dsatur` or `random` |
| `random_seed` | Random seed for coloring | Integer (-1 for dsatur) |
| `time_limit` | Maximum runtime in seconds | Positive integer |

### 🔵 Approach Options

- **`SS`** - San Segundo et al. bound ([EJOR 2019](https://doi.org/10.1016/j.ejor.2019.03.047))
- **`SH`** - Shimizu et al. bound ([Discrete Optimization 2020](https://doi.org/10.1016/j.disopt.2020.100583))
- **`HFB`** - Hosseinian et al. bound ([IJOC 2020](https://doi.org/10.1287/ijoc.2019.0898))

### 🎨 Coloring Method Options

- **`dsatur`** - DSATUR algorithm (recommended)
- **`random`** - Random coloring method

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

## 🔧 Example Usage

### Basic Example

```bash
./EWMCP_BOUNDS ./brock200_1.clq ./brock200_1.clq.weights SS dsatur -1 3600
```

This command:
- Uses graph `brock200_1.clq`
- Uses weights from `brock200_1.clq.weights`
- Applies San Segundo bound (`SS`)
- Uses DSATUR coloring
- No random seed needed (`-1`)
- Sets 1 hour time limit



## 📤 Output Format

The program writes results to **`results.txt`**. Each line contains:

###  Common Fields (All Approaches)
| Field | Description |
|-------|-------------|
| Graph file | Path to input graph |
| Weights file | Path to weights file |
| Approach | `SS`, `SH` or `HFB`|
| Coloring method | `dsatur` or `random` |
| Random seed | Seed value used |
| Time limit | Time limit in seconds |
| Vertices | Number of graph vertices |
| Edges | Number of graph edges |

### 🔵 San Segundo Bound (`SS`) Specific Fields
| Field | Description |
|-------|-------------|
| `cplex.getObjValue()` | CPLEX objective value |
| `cplex.getBestObjValue()` | CPLEX best bound |
| Solution status | CPLEX solution status |
| Computation time | Runtime in seconds |

### 🔴 Shimizu Bound (`SH`) Specific Fields
| Field | Description |
|-------|-------------|
| Bound (Policy 2) | Second policy bound value |
| Bound (Policy 1) | First policy bound value |
| Solution status | Algorithm status |
| Computation time | Runtime in seconds |

### 🟢 HFB Bound (`HFB`) Specific Fields
| Field | Description |
|-------|-------------|
| Bound value | HFB bound value |
| Solution status | Algorithm status |
| Computation time | Runtime in seconds |


---

<div align="center">
<sub>For more information, see the main <a href="README.md">README</a> or the <a href="additional_results.md">additional results</a></sub>
</div>
