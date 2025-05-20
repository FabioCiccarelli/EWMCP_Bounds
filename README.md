# Upper Bounds for the Edge-Weighted Maximum Clique Problem

This repository contains the implementation of two upper bounds for the Edge-Weighted Maximum Clique Problem (EWMCP), specifically the ones proposed by [San Segundo et al.](https://doi.org/10.1016/j.ejor.2019.03.047) and [Shimizu et al.](https://doi.org/10.1016/j.disopt.2020.100583). 

The code is associated with the research paper:

**"Strength of the upper bounds for the edge-weighted maximum clique problem"**  
by F. Ciccarelli, V. Dose, F. Furini and M. Monaci.


## Usage

The executable generated from this code takes the following parameters:

```
./EWMCP_BOUNDS <graph_file> <weights_file> <approach> <coloring_method> <random_seed> <time_limit>
```

### Parameters:

- **graph_file**: Path to the input graph file
- **weights_file**: Path to the edge weights file
- **approach**: Bounding approach to use
  - `SS` - San Segundo et al. bound
  - `SH` - Shimizu et al. bound
- **coloring_method**: Method for coloring the graph
  - `dsatur` - DSatur algorithm
  - `random` - Random coloring
- **random_seed**: Integer for random number generation (only used for random coloring, just put -1 in case of dsatur)
- **time_limit**: Time limit in seconds


#### DIMACS Graph Format

The code reads graph instances in the standard DIMACS format:

- `p edge n m ` - Problem line: n vertices, m edges
- `e i j` - Edge between vertices i and j, one for each line of the file

The weights file should instead contain one weight per line, matching the order of edges in the DIMACS file.

An example graph instance (`brock200_1.clq`) and its associated weights file (`brock200_1.clq.weights`) are provided in the repo.

### Example:
```
./EWMCP_BOUNDS ./brock200_1.clq ./brock200_1.clq.weights SS dsatur -1 3600
```

## Output

The program writes its results to a file named `results.txt`. Each line in this file contains:

- Graph instance file path
- Weights file path
- Bounding approach used (SS or SH)
- Coloring method used (dsatur or random)
- Random seed value
- Time limit value
- Number of vertices of the graph
- Number of edges of the graph

Followed by approach-specific values:

For San Segundo et al. bound (SS):
- Value of the bound
- Upper bound to the value of the bound
- Solution status
- Computation time

For Shimizu bound (SH):
- Value of the bound (second policy) 
- Value of the bound (first policy) 
- Solution status
- Computation time


## Acknowledgments

We would like to thank the contributors of [Brian Crites' graph-coloring repository](https://github.com/brrcrites/graph-coloring/tree/master) for the implementation of the DSATUR algorithm that we have included in our code.

## License

The software is for academic purposes only, see also the file license.md provided. To compile the code it is necessary to provide the path to the CPLEX directories.
