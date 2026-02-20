/**@file   graph.h
 * @brief Graph data structure for the Edge-Weighted Maximum Clique Problem
 * @author Fabio Ciccarelli
 *
 * This file defines a graph structure that contains all graph-related data
 * and operations needed for the EWMCP upper bounds.
*/

#ifndef GRAPH_H
#define GRAPH_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <string>

using namespace std;

/**
 * @brief Graph structure for undirected graphs with edge weights
 * 
 * Stores the graph using:
 * - Edge list (tail/head arrays, with tail[e] < head[e])
 * - Edge weights
 * - Adjacency matrix for O(1) edge queries
 * - Adjacency lists for neighbor iteration
 * - Per-node edge index arrays (parallel to adj_lists) for edge-based iteration
 */
struct Graph {
    int nnodes;                ///< Number of nodes in the graph
    int nedges;                ///< Number of edges in the graph

    int* tail;                 ///< tail[e] = tail node of edge e (0-indexed, tail < head)
    int* head;                 ///< head[e] = head node of edge e (0-indexed, tail < head)
    double* edge_weights;      ///< edge_weights[e] = weight of edge e

    bool** adj_matrix;         ///< Adjacency matrix: adj_matrix[i][j] = true if edge (i,j) exists
    int** adj_lists;           ///< Adjacency lists: adj_lists[i][k] = k-th neighbor of node i
    int** adj_edge_idx;        ///< Edge index lists: adj_edge_idx[i][k] = edge index of (i, adj_lists[i][k])
    int* node_degree;          ///< Degree of each node (also the size of adj_lists[i])

    /**
     * @brief Default constructor - creates an empty graph
     */
    Graph();

    /**
     * @brief Constructor - creates a graph from an edge list
     * @param n_nodes   number of nodes
     * @param n_edges   number of edges
     * @param edge_tails array of edge tail nodes (0-indexed)
     * @param edge_heads array of edge head nodes (0-indexed)
     * @param e_weights  array of edge weights (NULL for zero-initialized)
     * @param copy_data  if true, copies all input arrays; if false, takes ownership
     */
    Graph(int n_nodes, int n_edges, int* edge_tails, int* edge_heads,
          double* e_weights = nullptr, bool copy_data = true);

    /**
     * @brief Destructor - frees all allocated memory
     */
    ~Graph();

    /**
     * @brief Copy constructor
     */
    Graph(const Graph& other);

    /**
     * @brief Assignment operator
     */
    Graph& operator=(const Graph& other);

    /**
     * @brief Checks if an edge exists between two nodes
     * @param u first node (0-indexed)
     * @param v second node (0-indexed)
     * @return true if edge exists, false otherwise
     */
    bool hasEdge(int u, int v) const;

    /**
     * @brief Gets the neighbors of a node
     * @param node node index (0-indexed)
     * @param num_neighbors output parameter for number of neighbors
     * @return pointer to array of neighbor indices
     */
    const int* getNeighbors(int node, int& num_neighbors) const;

    /**
     * @brief Gets the degree of a node
     * @param node node index (0-indexed)
     * @return degree of the node
     */
    int getDegree(int node) const;

    /**
     * @brief Reads edge weights from a file (one weight per line, in edge order)
     * @param filename path to the weights file
     */
    void readEdgeWeights(const char* filename);

private:
    /**
     * @brief Builds the adjacency matrix from edge list
     */
    void buildAdjacencyMatrix();

    /**
     * @brief Builds adjacency lists, edge-index lists, and computes node degrees
     */
    void buildAdjacencyLists();

    /**
     * @brief Frees all allocated memory
     */
    void free();

    /**
     * @brief Deep copy from another graph
     */
    void copyFrom(const Graph& other);
};

/**
 * @brief Reads a graph from a DIMACS format file
 * @param inFile   path to the DIMACS file
 * @param nodes    output: number of nodes
 * @param edges    output: number of edges
 * @param tails    output: array of edge tails (must be pre-allocated)
 * @param heads    output: array of edge heads (must be pre-allocated)
 * @param checks   if true, performs undirectedness/connectivity checks (not recommended)
 */
void ReadDIMACSFile(char* inFile, int* nodes, int* edges, int* tails, int* heads, bool checks);

#endif // GRAPH_H
