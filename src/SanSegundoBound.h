#ifndef SanSegundoBound_HEADER
#define SanSegundoBound_HEADER

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <string.h>
#include <errno.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <set>
#include <iomanip> // for precision

#include "global_functions.h"
#include "global_variables.h"

#include "Graph_v4.h"

//#include "mnts.h"

using namespace std;


/***************************************************************************/
void SanSegundoBound_ModelBuild(instance *inst);
/***************************************************************************/

/***************************************************************************/
void SanSegundoBound_ModelSolve(instance *inst);
/***************************************************************************/

/***************************************************************************/
void SanSegundoBound_Free(instance *inst);
/***************************************************************************/


#endif
