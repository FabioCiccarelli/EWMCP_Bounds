# Upper Bounds for the Edge-Weighted Maximum Clique Problem

<div align="center">

[![License](https://img.shields.io/badge/License-Academic-blue.svg)](license.md)
[![C++](https://img.shields.io/badge/C++-17-blue.svg)](https://www.cplusplus.com/)
[![CPLEX](https://img.shields.io/badge/Requires-CPLEX-orange.svg)](https://www.ibm.com/products/ilog-cplex-optimization-studio)

</div>

<p align="center">
  <img src="EWMCP_Bounds_logo.png" alt="EWMCP logo" width="600"/>
</p>

## 📖 Overview

This repository contains the C++ implementation of two state-of-the-art upper bounds for the Edge-Weighted Maximum Clique Problem (EWMCP):

- 🔵 **San Segundo et al. bound** ([EJOR 2019](https://doi.org/10.1016/j.ejor.2019.03.047))
- 🔴 **Shimizu et al. bound** ([Discrete Optimization 2020](https://doi.org/10.1016/j.disopt.2020.100583))

## 📝 Associated Research

The code is associated with the research paper:

> **"Strength of the Upper Bounds for the Edge-Weighted Maximum Clique Problem"**  
> by *F. Ciccarelli, V. Dose, F. Furini and M. Monaci*
>
> 📄 [**Read the article**](https://arxiv.org/abs/2507.06898)

## 🚀 Quick Start

### Prerequisites
- C++ compiler with C++17 support
- IBM CPLEX Optimization Studio
- CMake (recommended)

## 📁 Repository Structure

- **[`src/`](src/)**: Source code implementation
- **[`INSTANCES/`](INSTANCES/)**: Benchmark instance dataset
  - [`INSTANCES/DIMACS.zip`](INSTANCES/DIMACS.zip): Standard DIMACS instances
  - [`INSTANCES/RANDOM.zip`](INSTANCES/RANDOM.zip): Randomly generated graph instances 

- [`TABLES/*.png`](TABLES/): Result tables and performance comparison charts referenced in [`additional_results.md`](additional_results.md)

- **Documentation**:
  - [`README.md`](README.md): Main project documentation (this file)
  - [`code_usage.md`](code_usage.md): Compilation and usage instructions
  - [`additional_results.md`](additional_results.md): Supplementary computational results and analysis
  - [`license.md`](license.md): Academic license terms and conditions

<br>

### 🏃‍♂️ Running the Code

For detailed compilation and usage instructions, see [`code_usage.md`](code_usage.md).

**Basic usage:**
```bash
./EWMCP_BOUNDS <graph_file> <weights_file> <approach> <coloring_method> <random_seed> <time_limit>
```

**Example:**
```bash
./EWMCP_BOUNDS ./brock200_1.clq ./brock200_1.clq.weights SS dsatur -1 3600
```

## 📊 Results

Comprehensive computational results and analysis can be found in [`additional_results.md`](additional_results.md).

## 🙏 Acknowledgments

We would like to thank the contributors of [Brian Crites' graph-coloring repository](https://github.com/brrcrites/graph-coloring/tree/master) for the implementation of the DSATUR algorithm that we have included in our code.


## 📜 License and Citation

This software is for academic purposes only. See [`license.md`](license.md) for full license details.

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

---

<br>

For further information, please contact: <f.ciccarelli@uniroma1.it>

<div align="center">
<sub> </sub>
</div>
