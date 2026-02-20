/**@file   graph.cpp
 * @brief  Graph data structure for the Edge-Weighted Maximum Clique Problem
 * @author Fabio Ciccarelli
 */

#include "graph.h"
#include <vector>
#include <algorithm>

using namespace std;

// ============================================================================
// Graph Constructor/Destructor and Memory Management
// ============================================================================

Graph::Graph()
    : nnodes(0), nedges(0), tail(nullptr), head(nullptr),
      edge_weights(nullptr), adj_matrix(nullptr), adj_lists(nullptr),
      adj_edge_idx(nullptr), node_degree(nullptr)
{
}

Graph::Graph(int n_nodes, int n_edges, int* edge_tails, int* edge_heads,
             double* e_weights, bool copy_data)
    : nnodes(n_nodes), nedges(n_edges)
{
    // Handle edge arrays
    if (copy_data) {
        tail = new int[nedges];
        head = new int[nedges];
        memcpy(tail, edge_tails, nedges * sizeof(int));
        memcpy(head, edge_heads, nedges * sizeof(int));
    } else {
        tail = edge_tails;
        head = edge_heads;
    }

    // Handle edge weights
    if (e_weights != nullptr) {
        if (copy_data) {
            edge_weights = new double[nedges];
            memcpy(edge_weights, e_weights, nedges * sizeof(double));
        } else {
            edge_weights = e_weights;
        }
    } else {
        // Initialize edge weights to zero
        edge_weights = new double[nedges];
        memset(edge_weights, 0, nedges * sizeof(double));
    }

    // Initialize pointers
    adj_matrix = nullptr;
    adj_lists = nullptr;
    adj_edge_idx = nullptr;
    node_degree = nullptr;

    // Build data structures
    buildAdjacencyMatrix();
    buildAdjacencyLists();
}

Graph::~Graph()
{
    free();
}

Graph::Graph(const Graph& other)
    : nnodes(0), nedges(0), tail(nullptr), head(nullptr),
      edge_weights(nullptr), adj_matrix(nullptr), adj_lists(nullptr),
      adj_edge_idx(nullptr), node_degree(nullptr)
{
    copyFrom(other);
}

Graph& Graph::operator=(const Graph& other)
{
    if (this != &other) {
        free();
        copyFrom(other);
    }
    return *this;
}

void Graph::free()
{
    if (tail != nullptr) {
        delete[] tail;
        tail = nullptr;
    }

    if (head != nullptr) {
        delete[] head;
        head = nullptr;
    }

    if (edge_weights != nullptr) {
        delete[] edge_weights;
        edge_weights = nullptr;
    }

    if (adj_matrix != nullptr) {
        for (int i = 0; i < nnodes; i++) {
            delete[] adj_matrix[i];
        }
        delete[] adj_matrix;
        adj_matrix = nullptr;
    }

    if (adj_lists != nullptr) {
        for (int i = 0; i < nnodes; i++) {
            if (adj_lists[i] != nullptr) {
                delete[] adj_lists[i];
            }
        }
        delete[] adj_lists;
        adj_lists = nullptr;
    }

    if (adj_edge_idx != nullptr) {
        for (int i = 0; i < nnodes; i++) {
            if (adj_edge_idx[i] != nullptr) {
                delete[] adj_edge_idx[i];
            }
        }
        delete[] adj_edge_idx;
        adj_edge_idx = nullptr;
    }

    if (node_degree != nullptr) {
        delete[] node_degree;
        node_degree = nullptr;
    }
}

void Graph::copyFrom(const Graph& other)
{
    nnodes = other.nnodes;
    nedges = other.nedges;

    // Copy edge arrays
    if (other.tail != nullptr) {
        tail = new int[nedges];
        memcpy(tail, other.tail, nedges * sizeof(int));
    }

    if (other.head != nullptr) {
        head = new int[nedges];
        memcpy(head, other.head, nedges * sizeof(int));
    }

    // Copy edge weights
    if (other.edge_weights != nullptr) {
        edge_weights = new double[nedges];
        memcpy(edge_weights, other.edge_weights, nedges * sizeof(double));
    }

    // Copy adjacency matrix
    if (other.adj_matrix != nullptr) {
        adj_matrix = new bool*[nnodes];
        for (int i = 0; i < nnodes; i++) {
            adj_matrix[i] = new bool[nnodes];
            memcpy(adj_matrix[i], other.adj_matrix[i], nnodes * sizeof(bool));
        }
    }

    // Copy node degrees
    if (other.node_degree != nullptr) {
        node_degree = new int[nnodes];
        memcpy(node_degree, other.node_degree, nnodes * sizeof(int));
    }

    // Copy adjacency lists
    if (other.adj_lists != nullptr && other.node_degree != nullptr) {
        adj_lists = new int*[nnodes];
        for (int i = 0; i < nnodes; i++) {
            if (node_degree[i] > 0) {
                adj_lists[i] = new int[node_degree[i]];
                memcpy(adj_lists[i], other.adj_lists[i], node_degree[i] * sizeof(int));
            } else {
                adj_lists[i] = nullptr;
            }
        }
    }

    // Copy edge index lists
    if (other.adj_edge_idx != nullptr && other.node_degree != nullptr) {
        adj_edge_idx = new int*[nnodes];
        for (int i = 0; i < nnodes; i++) {
            if (node_degree[i] > 0) {
                adj_edge_idx[i] = new int[node_degree[i]];
                memcpy(adj_edge_idx[i], other.adj_edge_idx[i], node_degree[i] * sizeof(int));
            } else {
                adj_edge_idx[i] = nullptr;
            }
        }
    }
}

void Graph::buildAdjacencyMatrix()
{
    // Allocate adjacency matrix
    adj_matrix = new bool*[nnodes];
    for (int i = 0; i < nnodes; i++) {
        adj_matrix[i] = new bool[nnodes];
        memset(adj_matrix[i], 0, nnodes * sizeof(bool));
    }

    // Fill adjacency matrix from edge list
    for (int e = 0; e < nedges; e++) {
        int u = tail[e];
        int v = head[e];

        adj_matrix[u][v] = true;
        adj_matrix[v][u] = true;
    }
}

void Graph::buildAdjacencyLists()
{
    // Initialize node degrees
    node_degree = new int[nnodes];
    memset(node_degree, 0, nnodes * sizeof(int));

    // Count degrees
    for (int e = 0; e < nedges; e++) {
        node_degree[tail[e]]++;
        node_degree[head[e]]++;
    }

    // Allocate adjacency lists and edge index lists
    adj_lists = new int*[nnodes];
    adj_edge_idx = new int*[nnodes];
    vector<int> current_sizes(nnodes, 0);

    for (int i = 0; i < nnodes; i++) {
        if (node_degree[i] > 0) {
            adj_lists[i] = new int[node_degree[i]];
            adj_edge_idx[i] = new int[node_degree[i]];
        } else {
            adj_lists[i] = nullptr;
            adj_edge_idx[i] = nullptr;
        }
    }

    // Fill adjacency lists and edge index lists
    for (int e = 0; e < nedges; e++) {
        int u = tail[e];
        int v = head[e];

        adj_lists[u][current_sizes[u]] = v;
        adj_edge_idx[u][current_sizes[u]] = e;
        current_sizes[u]++;

        adj_lists[v][current_sizes[v]] = u;
        adj_edge_idx[v][current_sizes[v]] = e;
        current_sizes[v]++;
    }
}

// ============================================================================
// Graph Query Methods
// ============================================================================

bool Graph::hasEdge(int u, int v) const
{
    if (u < 0 || u >= nnodes || v < 0 || v >= nnodes) {
        return false;
    }
    return adj_matrix[u][v];
}

const int* Graph::getNeighbors(int node, int& num_neighbors) const
{
    if (node < 0 || node >= nnodes) {
        num_neighbors = 0;
        return nullptr;
    }
    num_neighbors = node_degree[node];
    return adj_lists[node];
}

int Graph::getDegree(int node) const
{
    if (node < 0 || node >= nnodes) {
        return 0;
    }
    return node_degree[node];
}

// ============================================================================
// Edge Weights I/O
// ============================================================================

void Graph::readEdgeWeights(const char* filename)
{
    ifstream in(filename);
    if (!in) {
        cout << "Weights file could not be opened: " << filename << endl;
        exit(1);
    }

    for (int i = 0; i < nedges; i++) {
        in >> edge_weights[i];
    }

    in.close();
}

// ============================================================================
// DIMACS File Reader
// ============================================================================

void ReadDIMACSFile(char* inFile, int* nodes, int* edges, int* tails, int* heads, bool checks)
{
    cout << "DIMACS INSTANCE\n";
    cout << "\n-> original graph\n";

    ifstream in(inFile);
    if (!in) {
        cout << "File could not be opened. " << endl;
        exit(1);
    }

    string str1;
    string str2 = "e";
    string str3 = "p";
    char buf_1[5000];

    for (string line; getline(in, line);) {
        istringstream buffer(line);
        buffer >> str1;

        if (!str3.compare(str1)) {
            buffer >> buf_1;
            buffer >> *nodes;
            buffer >> *edges;
            break;
        }
    }

    int i = 0;
    for (string line; getline(in, line);) {
        istringstream buffer(line);
        buffer >> str1;

        if (!str2.compare(str1)) {
            int a, b;
            buffer >> a;
            buffer >> b;

            tails[i] = min(a - 1, b - 1);
            heads[i] = max(a - 1, b - 1);

            if (i == (*edges - 1)) {
                break;
            }
            i++;
        }
    }

    in.close();

    cout << "read!\n";
}
