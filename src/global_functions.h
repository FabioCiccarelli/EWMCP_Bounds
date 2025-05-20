#ifndef GLOBAL_FUNCTIONS_HEADER
#define GLOBAL_FUNCTIONS_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <random>
#include <vector>
#include <algorithm>


#include "global_variables.h"
#include "coloring_algorithm.hpp"
#include "dsatur.hpp"

using namespace std;
using GraphColoring::Dsatur;


/***************************************************************************/
void generate_instances();
/***************************************************************************/

/***************************************************************************/
void generate_edge_weights(instance *inst);
/***************************************************************************/

/***************************************************************************/
void write_edge_weights_file(instance *inst);
/***************************************************************************/

/***************************************************************************/
void ShimizuBound(instance *inst);
/***************************************************************************/

/***************************************************************************/
void ShimizuBound_first_policy(instance *inst);
/***************************************************************************/

/***************************************************************************/
void dsatur_color(instance *inst);
/***************************************************************************/

/***************************************************************************/
void random_color(instance *inst);
/***************************************************************************/



#endif
