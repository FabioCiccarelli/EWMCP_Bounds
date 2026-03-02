#ifndef TSM_MWC_HEADER
#define TSM_MWC_HEADER


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <sys/resource.h>
#include <math.h>
#include <assert.h>

#include "global_variables.h"



/////////////////////////////////////// STATIC VARIABLES ///////////////////////////////////////

#define WORD_LENGTH 100
#define TRUE 1
#define FALSE 0
#define NONE -1
#define DELIMITER 0
#define PASSIVE 0
#define ACTIVE 1
#define UNASSIGNED 2
#define P_TRUE 1
#define P_FALSE 0
#define NULL_REASON -1
#define NO_REASON -3
#define CONFLICT -1978
#define MAX_NODE 10000000
#define max_expand_depth 100000
#define STACK_LENGTH (MAX_NODE*2)
#define pop(stack) stack[--stack ## _fill_pointer]
#define push(item, stack) stack[stack ## _fill_pointer++] = item
#define ptr(stack) stack ## _fill_pointer
#define is_neibor(i,j) matrice[i][j]

#define CUR_CLQ_SIZE Clique_Stack_fill_pointer
#define CURSOR Cursor_Stack[Cursor_Stack_fill_pointer-1]
#define MIN(a,b) a<=b?a:b
#define BIT_MAP_SIZE 4097
#define Node_Reason Node_Degree

#define SET_EDGE(row,col) ((*(Adj_Matrix + (row)* MATRIX_ROW_WIDTH + ((col) >> 3))) |= (1 << ((col) & 7)))
#define GET_EDGE(row,col) ((*(Adj_Matrix + (row)* MATRIX_ROW_WIDTH + ((col) >> 3))) & (1 << ((col) & 7)))

#define iMatrix(i) (Adj_Matrix+(i)*MATRIX_ROW_WIDTH)
#define Matrix(i,j) ((*((i) + ((j) >> 3))) & (1 << ((j) & 7)))

static int * Adj_List;
#define New_Name Node_Degree

static int FORMAT = 1, NB_NODE, NB_NODE_O, NB_EDGE, NB_EDGE_O, MAX_CLQ_SIZE, INIT_CLQ_SIZE, INIT_ORDERING, NB_BACK_CLIQUE, MATRIX_ROW_WIDTH, MATRIX_SIZE = 0, MAX_SUBGRAPH_SIZE, K_CORE_G = 0;

static long long MAX_CLQ_WEIGHT, CUR_CLQ_WEIGHT, INIT_CLQ_WEIGHT, OPT_CLQ_WEIGHT, MAX_ISET_WEIGHT, CUR_NODE_WEIGHT, UPPER_WEIGHT_BOUND;
static long long Max_Degree = 0;
static long long Node_Degree[MAX_NODE];
//static int Max_Weight = 0;
static long long Top_Weight[MAX_NODE];
static long long Node_Weight[MAX_NODE];
static char Node_Value[MAX_NODE];
static int **Node_Neibors;

static int Candidate_Stack_fill_pointer = 0;
static long long Candidate_Stack[MAX_NODE * 2];
static long long Vertex_UB[MAX_NODE * 2];
static int Clique_Stack_fill_pointer;
static int *Clique_Stack, *MaxCLQ_Stack;
static int Cursor_Stack[max_expand_depth];
static int Cursor_Stack_fill_pointer = 0;
static int Weight_Mod = 200;
static unsigned char * Adj_Matrix;

static int iSET_COUNT = 0;
static long long iSET_TOTAL_WEIGHT = 0;
static int *iSET_Size;
static long long *iSET_Weight;
static char *iSET_Tested;
static long long **iSET;
static int NEW_NODE_IDX = 0, MAX_OLD_NODE = 0, MAX_ISET_COUNT = 0;
static int RESERVED_LENGTH = 100;
static int LARGE_WEIGHT = FALSE;


/* the structures for maxsat reasoning*/

static struct iSET_State *IS;
static long long *iNode_TAIL;

struct iSET_State {
	char satisfied;
	char used;
	char involved;
	char active;
	int size;
	int topk;
	long long weight;  //int unassigned;
	long long t_weight;  //int unassigned;
	long long *nodes;
};

static long long *REASON_STACK;
static int REASON_STACK_fill_pointer = 0;
static int *UNIT_STACK;
static int UNIT_STACK_fill_pointer = 0;
//static int *TOP_UNIT_STACK;
//static int TOP_UNIT_STACK_fill_pointer = 0;
static int *NEW_UNIT_STACK;
static int NEW_UNIT_STACK_fill_pointer = 0;
static int *FIXED_NODES_STACK;
static int FIXED_NODES_STACK_fill_pointer = 0;
static int *SATISFIED_iSETS_STACK;
static int SATISFIED_iSETS_STACK_fill_pointer = 0;
static int *TOPK_REDUCED_STACK;
static int TOPK_REDUCED_STACK_fill_pointer = 0;

static int Rollback_Point;
static int Branching_Point;
static int *Old_Name;
static int *Second_Name;
static int NB_CANDIDATE = 0, FIRST_INDEX;
static int START_MAXSAT_THD = 15;
static int BRANCHING_COUNT = 0;
static int CUR_MAX_NODE;

static int Last_Idx = 0;
static int cut_ver = 0, total_cut_ver = 0;
static int cut_inc = 0, total_cut_inc = 0;
static int cut_iset = 0, total_cut_iset = 0;
static int cut_satz = 0, total_cut_satz = 0;

static int LAST_IN;
static int SEARCH_UPDATED_CLIQUE = 0;  // set to 1 when search updates MaxCLQ_Stack

/*statistical data*/
static long long N0_B, N0_A, N1_B, G1_B = 0;
static double D0_B, D0_A, D1_B = 0;
/*****************/
static int * Init_Adj_List;
static int BLOCK_COUNT = 0;
static int *BLOCK_LIST[100];
static double READ_TIME, INIT_TIME, SEARCH_TIME;

/////////////////////////////////////// TIME LIMIT ///////////////////////////////////////

#include <chrono>
extern int TSM_TIME_LIMIT_HIT;
extern chrono::steady_clock::time_point TSM_wall_start;
extern double TSM_TIME_LIMIT_SEC;

void set_TSM_time_limit(double seconds);
bool TSM_check_time_limit();

/////////////////////////////////////// FUNCTIONS ///////////////////////////////////////

/***************************************************************************/
int init_TSM(instance *inst);
/***************************************************************************/

/***************************************************************************/
int build_TSM_graph_instance(instance *inst);
/***************************************************************************/

/***************************************************************************/
int TSM_MWC(instance *inst);
/***************************************************************************/

/***************************************************************************/
void free_TSM();
/***************************************************************************/
#endif