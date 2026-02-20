# 📊 Additional Computational Results

<div align="center">

[![Article](https://img.shields.io/badge/Full_Article-Available-green.svg)](https://arxiv.org/abs/2507.06898)


</div>

---

## 📋 Overview

This document contains additional numerical results that supplement those presented in the main paper. These extended results provide insights into the performance and behavior of the proposed upper bounds.

## 📖 Table Structure Reference

### 📌 Column Definitions

The following column headers are shared across multiple tables and are defined here for reference:

| Column | Description |
|--------|-------------|
| **\|V\|** | Number of vertices in the graph |
| **% μ** | Percentage edge density of the graph |
| **#** | Number of instances sharing the same parameters |
| **% gap** | Percentage gap between bound and actual edge-weighted clique number |
| **% diff.** | Percentage difference between the two bounds |

<br>

The percentage difference between bounds is computed as:

$$\% \text{diff.} = 100 \cdot \frac{UB_1 - UB_2}{\max\{UB_1, \, UB_2\}}$$

---

## 📈 Computational Results

### 🔍 Bound Comparison on Variable Graph Sizes

<div align="center">

**Table 1:** *Comparison of UB₁ and UB₂ on random graphs with varying number of vertices*

![Random graphs with variable density and nodes](TABLES/table_random_nodes.png)

</div>

---

### 📊 Density Impact Analysis

<div align="center">

**Table 2:** *Comparison of UB₁ and UB₂ on random graphs with |V| = 100 and varying density*

![Random graphs with fixed n=100 and variable density](TABLES/table_random_densities_n100.png)

</div>

---

## 🔄 Independent Set Ordering Sensitivity Analysis

The following tables present numerical results aimed at studying the variation of UB₂ values across different orderings of independent sets within the same coloring (specifically, obtained through DSATUR). 

The reported statistics analyze how the minimum and maximum bound values deviate from the average across these different orderings.

### Variable Graph Sizes

<div align="center">

**Table 3:** *UB₂ variation across 10 different independent set orderings in graph coloring*

![Random graphs with variable independent shuffling](TABLES/table_random_shuffling.png)

</div>

---

### Fixed Graph Size (|V| = 100)

<div align="center">

**Table 4:** *UB₂ variation across 10 different independent set orderings in graph coloring (|V| = 100)*

![Random graphs with variable independent shuffling n 100](TABLES/table_random_shuffling_n100.png)

</div>

---

## 🎨 Coloring Method Sensitivity Analysis

The following tables demonstrate how the **average gap** between upper bound values and the optimal edge-weighted clique number is sensitive to the chosen **coloring method**.

### Multiple Coloring Methods Comparison

<div align="center">

**Table 5:** *Average percentage gap of UB₁ and UB₂ across different colorings*

![Random graphs with different colorings](TABLES/table_random_coloring.png)

</div>

---

### Fixed Graph Size Analysis

<div align="center">

**Table 6:** *Average percentage gap of UB₁ and UB₂ across different colorings (|V| = 100)*

![Random graphs with different colorings](TABLES/table_random_coloring_n100.png)

</div>

---

## 🎯 DIMACS Benchmark Analysis

The following table shows the percentage difference between the two upper bounds across DIMACS families, comparing results obtained with random edge weights versus those with deterministic edge weights. 

> **Note:** Since optimal solution values for EWMCP on instances with random edge weights are not available, gap-related statistics are omitted from this analysis.

<div align="center">

**Table 7:** *Percentage difference between UB₁ and UB₂ for DIMACS instances with random edge weights*

![DIMACS families with random weights](TABLES/table_dimacs_families_randw.png)

</div>

---

---

<div align="center">
<sub>For implementation details, see the <a href="code_usage.md">code usage guide</a> | Return to <a href="README.md">main README</a></sub>
</div>
<br>
<br>

