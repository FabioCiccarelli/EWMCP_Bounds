# Upper Bounds for the Edge-Weighted Maximum Clique Problem

<div align="center">

[![License](https://img.shields.io/badge/License-Academic-blue.svg)](license.md)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://www.cplusplus.com/)
[![CPLEX](https://img.shields.io/badge/Requires-CPLEX-orange.svg)](https://www.ibm.com/products/ilog-cplex-optimization-studio)

</div>

<p align="center">
  <img src="EWMCP_Bounds_logo.png" alt="EWMCP logo" width="600"/>
</p>

## Overview

This repository contains a C++17 implementation of upper bounds for the Edge-Weighted Maximum Clique Problem (EWMCP):

- `SS` - San Segundo et al. bound ([San Segundo et al.](https://doi.org/10.1016/j.ejor.2019.03.047))
- `SSpooled` - San Segundo bound with pooled independent sets from DSATUR + 5 random colorings
- `SH` - Shimizu et al. bound ([Shimizu et al.](https://doi.org/10.1016/j.disopt.2020.100583))
- `HFB` - Hosseinian et al. bound ([Hosseinian et al.](https://doi.org/10.1287/ijoc.2019.0898))
- `CG` - column-generation bound with independent-set separation
- `F1` and `F11` - linear relaxations of compact LP formulations for the EWMCP ([see Gouveia and Martins](https://doi.org/10.1007/s13675-014-0028-1))

The code is associated with the paper "Strength of the Upper Bounds for the Edge-Weighted Maximum Clique Problem" by F. Ciccarelli, V. Dose, F. Furini and M. Monaci.

## Build And Run

Prerequisites:

- C++ compiler with C++17 support
- IBM ILOG CPLEX
- `CPLEX_PATH` exported so that the [Makefile](Makefile) can resolve headers and static libraries

Build from the repository root:

```bash
make
```

The executable is generated as `./bin/EWMCP_BOUNDS`.

Basic usage:

```bash
./bin/EWMCP_BOUNDS path/to/instance.clq --bound SH --coloring dsatur
```

The weights file is derived automatically as `path/to/instance.clq.weights`.

Examples in this README intentionally use generic instance paths: benchmark folders and experimental workspaces used locally are not part of the tracked repository.

More details:

- [docs/codeUsage.md](docs/codeUsage.md) - build, CLI, input and output format
- [docs/branchingStudy.md](docs/branchingStudy.md) - single-branch branching study mode

## Repository Layout

- [src/](src/) - core implementation of the bounds, coloring, graph handling and CLI
- [docs/](docs/) - user-facing documentation
- [data/instances/](data/instances/) - benchmark archives, including [DIMACS](data/instances/DIMACS.zip) and [RANDOM](data/instances/RANDOM.zip) instances
- [data/results/](data/results/) - Excel workbooks with instance-wise computational results
- [Makefile](Makefile) - build entry point
- [license.md](license.md) - academic license terms

## Result Workbooks

The public result summaries currently stored in [data/results/](data/results/) are:

| File | Description |
| --- | --- |
| [data/results/boundComparisonDIMACS.xlsx](data/results/boundComparisonDIMACS.xlsx) | Instance-wise comparison of `SS`, `SH` and `HFB` on DIMACS, with Dsatur and five random colorings. |
| [data/results/boundComparisonRANDOM.xlsx](data/results/boundComparisonRANDOM.xlsx) | Instance-wise comparison of `SS`, `SH` and `HFB` on RANDOM instances, with Dsatur and five random colorings. |
| [data/results/LP-basedBounds.xlsx](data/results/LP-basedBounds.xlsx) | Comparison of LP-based upper bounds (`SS`, `SSpooled`, `CG`, `F1` and `F11`) on DIMACS instances with known optimum. |
| [data/results/sortingStudyUB2.xlsx](data/results/sortingStudyUB2.xlsx) | Sorting-strategy study for the Shimizu bound, with per-instance DIMACS results. |
| [data/results/branchingSimulation.xlsx](data/results/branchingSimulation.xlsx) | Single-branch B&B simulation study for `SH`, `SS` and `HFB`, including summary and per-seed sheets. |
| [data/results/EWMCPalgoComparison.xlsx](data/results/EWMCPalgoComparison.xlsx) | Comparison of state-of-the-art exact EWMCP algorithms on DIMACS instances. |

## Acknowledgments

This repository includes a DSatur implementation adapted from Brian Crites' [graph-coloring](https://github.com/brrcrites/graph-coloring) codebase.

## License And Citation

This software is for academic use only. See [license.md](license.md) for details.

If you use it in academic work, please cite:

```bibtex
@misc{ciccarelli2025strengthupperboundsedgeweighted,
  title={Strength of the Upper Bounds for the Edge-Weighted Maximum Clique Problem},
  author={Fabio Ciccarelli and Valerio Dose and Fabio Furini and Marta Monaci},
  year={2025},
  eprint={2507.06898},
  archivePrefix={arXiv}
}
```

For further information: <f.ciccarelli@uniroma1.it>
