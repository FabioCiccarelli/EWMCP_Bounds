#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <float.h>

using namespace std;

#include "global_variables.h"
#include "global_functions.h"
#include "coloring_algorithm.hpp"
#include "dsatur.hpp"
#include "SanSegundoBound.h"
#include "ShimizuBound.h"
#include "HFBBound.h"
#include "CGBound.h"
#include "F11Bound.h"
#include "BranchingTest.h"


/************************************************************************************************/
int main(int argc, char** argv)
/************************************************************************************************/
{


	////////////////////
	// generate_instances();
	// exit(-1);
	////////////////////

	instance inst;

	inst.istname_graph=new char[2000];
	inst.istname_weights=new char[2000];

	// Default parameter values
	inst.PARAM_APPROACH = "";
	inst.PARAM_COLORING_METHOD = "dsatur";
	inst.PARAM_RANDOM_SEED = -1;
	inst.PARAM_TIME_LIMIT = 3600.0;
	inst.PARAM_SORTING_STRATEGY = "natural";
	inst.PARAM_SORTING_SENSE = 1;

	bool test_branching = false;
	double branching_incumbent = 0.0;

	if (argc < 2)
	{
		cout << "Usage: ./bin/EWMCP_BOUNDS <instance_path> --bound <SH|SS|SSpooled|HFB|CG|F1|F11> [options]" << endl;
		cout << "       ./bin/EWMCP_BOUNDS <instance_path> --bound <SH|SS|HFB> --test-branching --incumbent <value> [options]" << endl;
		cout << "Use --help for more information." << endl;
		exit(1);
	}

	string arg1 = argv[1];

	if (arg1 == "--help")
	{
		cout << "----------------------------------------------\n";
		cout << "             PROGRAM USAGE                    \n";
		cout << "----------------------------------------------\n";
		cout << "./bin/EWMCP_BOUNDS <instance_path> [options]\n\n";
		cout << "The weights file is derived automatically as <instance_path>.weights\n\n";
		cout << "Options:\n";
		cout << "  --bound <SH|SS|SSpooled|HFB|CG|F1|F11>  Bounding approach (required)\n";
		cout << "  --coloring <dsatur|random>        Coloring method (default: dsatur)\n";
		cout << "  --seed <int>                     Random seed (default: -1, used with random coloring)\n";
		cout << "  --time-limit <seconds>           Time limit in seconds (default: 3600)\n";
		cout << "  --sorting-strategy <natural|size|weight>\n";
		cout << "                                   Stable set sorting strategy for SH bound (default: natural)\n";
		cout << "  --sorting-sense <1|-1>           Sorting direction: 1=ascending, -1=descending (default: 1)\n";
		cout << "  --test-branching                 Run single-branch B&B simulation (SH, SS, HFB only)\n";
		cout << "  --incumbent <double>             Incumbent value for pruning (required with --test-branching)\n";
		cout << "----------------------------------------------\n";
		exit(0);
	}

	// First positional argument: instance path
	strcpy(inst.istname_graph, argv[1]);
	// Weights file derived automatically
	sprintf(inst.istname_weights, "%s.weights", inst.istname_graph);

	// Parse named arguments
	for (int i = 2; i < argc; i++)
	{
		string arg = argv[i];

		if (arg == "--bound" && i + 1 < argc)
		{
			inst.PARAM_APPROACH = argv[++i];
		}
		else if (arg == "--coloring" && i + 1 < argc)
		{
			inst.PARAM_COLORING_METHOD = argv[++i];
		}
		else if (arg == "--seed" && i + 1 < argc)
		{
			inst.PARAM_RANDOM_SEED = atoi(argv[++i]);
		}
		else if (arg == "--time-limit" && i + 1 < argc)
		{
			inst.PARAM_TIME_LIMIT = atof(argv[++i]);
		}
		else if (arg == "--sorting-strategy" && i + 1 < argc)
		{
			inst.PARAM_SORTING_STRATEGY = argv[++i];
		}
		else if (arg == "--sorting-sense" && i + 1 < argc)
		{
			inst.PARAM_SORTING_SENSE = atoi(argv[++i]);
		}
		else if (arg == "--test-branching")
		{
			test_branching = true;
		}
		else if (arg == "--incumbent" && i + 1 < argc)
		{
			branching_incumbent = atof(argv[++i]);
		}
		else
		{
			cout << "ERROR: Unrecognized option or missing value: " << arg << endl;
			exit(2);
		}
	}

	// Validate required parameters
	if (inst.PARAM_APPROACH.empty())
	{
		cout << "ERROR: --bound is required. Use --help for usage information." << endl;
		exit(2);
	}

	if (inst.PARAM_APPROACH != "SH" && inst.PARAM_APPROACH != "SS" && inst.PARAM_APPROACH != "SSpooled" && inst.PARAM_APPROACH != "HFB" && inst.PARAM_APPROACH != "CG" && inst.PARAM_APPROACH != "F1" && inst.PARAM_APPROACH != "F11")
	{
		cout << "ERROR: --bound must be SH, SS, SSpooled, HFB, CG, F1, or F11." << endl;
		exit(2);
	}

	if (test_branching)
	{
		if (inst.PARAM_APPROACH != "SH" && inst.PARAM_APPROACH != "SS" && inst.PARAM_APPROACH != "HFB")
		{
			cout << "ERROR: --test-branching only supports SH, SS, or HFB bounds." << endl;
			exit(2);
		}
		if (branching_incumbent <= 0.0)
		{
			cout << "ERROR: --incumbent is required with --test-branching (must be > 0)." << endl;
			exit(2);
		}
	}

	if (inst.PARAM_COLORING_METHOD != "dsatur" && inst.PARAM_COLORING_METHOD != "random")
	{
		cout << "ERROR: --coloring must be dsatur or random." << endl;
		exit(2);
	}

	if (inst.PARAM_SORTING_STRATEGY != "natural" && inst.PARAM_SORTING_STRATEGY != "size" && inst.PARAM_SORTING_STRATEGY != "weight")
	{
		cout << "ERROR: --sorting-strategy must be natural, size, or weight." << endl;
		exit(2);
	}

	if (inst.PARAM_SORTING_SENSE != 1 && inst.PARAM_SORTING_SENSE != -1)
	{
		cout << "ERROR: --sorting-sense must be 1 or -1." << endl;
		exit(2);
	}

	cout << "\n";
	cout << "istname_graph: ->\t" <<  inst.istname_graph << endl;
	cout << "istname_weights: ->\t" <<  inst.istname_weights << endl;

	cout << "PARAM_APPROACH: ->\t" <<  inst.PARAM_APPROACH << endl;
	cout << "PARAM_COLORING_METHOD: ->\t" <<  inst.PARAM_COLORING_METHOD << endl;

	cout << "PARAM_RANDOM_SEED: ->\t" <<  inst.PARAM_RANDOM_SEED << endl;
	cout << "PARAM_TIME_LIMIT: ->\t" <<  inst.PARAM_TIME_LIMIT << endl;
	cout << "PARAM_SORTING_STRATEGY: ->\t" <<  inst.PARAM_SORTING_STRATEGY << endl;
	cout << "PARAM_SORTING_SENSE: ->\t" <<  inst.PARAM_SORTING_SENSE << endl;
	if (test_branching)
	{
		cout << "TEST_BRANCHING: ->\tENABLED" << endl;
		cout << "INCUMBENT: ->\t" << branching_incumbent << endl;
	}


	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int nodes;
	int edges;

	int *heads=new int[100000000];
	int *tails=new int[100000000];

	ReadDIMACSFile(inst.istname_graph,&nodes,&edges,tails,heads,false);

	cout << "GRAPH BUILDING\n";
	inst.G = new Graph(nodes, edges, heads, tails);
	cout << "DONE\n";

	delete[] heads;
	delete[] tails;

	/////////////////////////////////////////////////
	inst.G->readEdgeWeights(inst.istname_weights);
	/////////////////////////////////////////////////

	cout << "\nVertices\t" << inst.G->nnodes << "\tEdges\t" << inst.G->nedges << endl;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////


	clock_t time_start;
	clock_t time_end;

	// --- Branching test mode ---
	if (test_branching)
	{
		cout << "\n************************************\n";
		cout << "SINGLE-BRANCH B&B SIMULATION (" << inst.PARAM_APPROACH << ")\n";
		cout << "************************************\n";

		BranchingTestResult bt_result = run_branching_test(&inst, branching_incumbent);
		write_branching_results(&inst, bt_result);

		cout << "\n\nDONE!!";
		cout << "\n************************************\n\n\n";

		delete [] inst.istname_graph;
		delete [] inst.istname_weights;
		delete inst.G;
		return 0;
	}

	if(inst.PARAM_APPROACH == "SH" || inst.PARAM_APPROACH == "SS" || inst.PARAM_APPROACH == "SSpooled" || inst.PARAM_APPROACH == "HFB" || inst.PARAM_APPROACH == "CG" || inst.PARAM_APPROACH == "F11" || inst.PARAM_APPROACH == "F1")	{

		// SSpooled handles coloring internally (dsatur + 5 random colorings)
		if(inst.PARAM_APPROACH != "SSpooled")
		{
			if(inst.PARAM_COLORING_METHOD == "dsatur")
			{
				cout << "\n************************************\n";
				cout << "DSATUR COLORING\n";
				dsatur_color(&inst);
			}

			else if(inst.PARAM_COLORING_METHOD == "random")
			{
				cout << "\n************************************\n";
				cout << "RANDOM COLORING\n";
				random_color(&inst);
			}

			cout << "Number of colors: " << inst.num_colors << endl;
		}

	#ifdef SHUFFLING_ANALYSIS
		suffling_analysis(&inst);
		exit(0);
	#endif

	
// for(int i=0;i<inst.G->nnodes;i++)
		// {
		// 	// print the color of each vertex
		// 	cout << "Vertex " << i+1 << " color: " << inst.v_color[i] + 1 << endl;
		// }

		
		if(inst.PARAM_APPROACH == "SH")
		{
			cout << "\n************************************\n";
			cout << "Shimizu BOUND\n";
			time_start=clock();
		
			ShimizuBound(&inst);
			
			time_end=clock();

			inst.ShimizuBound_Time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;

			cout << "Shimizu Bound time: " << inst.ShimizuBound_Time << endl;

			ShimizuBound_first_policy(&inst);

		}
		else if(inst.PARAM_APPROACH == "SS")
		{
			cout << "\n************************************\n";
			cout << "SAN SEGUNDO BOUND\n\n";
			SanSegundoBound_ModelBuild(&inst);

			time_start=clock();

			SanSegundoBound_ModelSolve(&inst);

			time_end=clock();
			inst.SanSegundoBound_Time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;

			SanSegundoBound_Free(&inst);

			cout << "SanSegundo Bound time: " << inst.SanSegundoBound_Time << endl;

		}
		else if(inst.PARAM_APPROACH == "SSpooled")
		{
			cout << "\n************************************\n";
			cout << "SAN SEGUNDO POOLED BOUND\n\n";

			time_start=clock();

			SanSegundoPooled(&inst);

			time_end=clock();
			inst.SanSegundoBound_Time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;

			cout << "SanSegundo Pooled Bound time: " << inst.SanSegundoBound_Time << endl;
		}
		else if(inst.PARAM_APPROACH == "HFB")
		{
			cout << "\n************************************\n";
			cout << "HFB BOUND\n";
			time_start=clock();

			HFBBound(&inst);

			time_end=clock();
			inst.HFBBound_Time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;

			cout << "HFB Bound time: " << inst.HFBBound_Time << endl;
		}
		else if(inst.PARAM_APPROACH == "CG")
		{
			cout << "\n************************************\n";
			cout << "CG BOUND\n";
			time_start=clock();

			CGBound_Solve(&inst);

			time_end=clock();
			inst.CGBound_Time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;

			cout << "CG Bound time: " << inst.CGBound_Time << endl;
		}
		else if(inst.PARAM_APPROACH == "F11" || inst.PARAM_APPROACH == "F1")
		{
			cout << "\n************************************\n";
			if(inst.PARAM_APPROACH == "F11") 
			{
			cout << "F11 BOUND\n";
			}
			else
			{
			cout << "F1 BOUND\n";
			}
			time_start=clock();

			F11Bound_Solve(&inst);

			time_end=clock();
			inst.F11Bound_Time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;

			if(inst.PARAM_APPROACH == "F11") 
			{
			cout << "F11 Bound time: " << inst.F11Bound_Time << endl;
			}
			else
			{
			cout << "F1 Bound time: " << inst.F11Bound_Time << endl;
			}
		}
		
		cout << "\n\nDONE!!";
		cout << "\n************************************\n\n\n";
	}
	
	else
	{
		cout << "UNRECOGNIZED BOUNDING APPROACH\n";
		exit(2);
	}


	ofstream info_SUMMARY("results.txt", ios::app);

	// Uniform output format: same columns for all approaches
	// Columns: instance | approach | coloring | seed | time_limit | sorting_strategy | sorting_sense |
	//          nnodes | nedges | num_colors | bound_value | bound_value_2 | status | time

	string approach_label = inst.PARAM_APPROACH;

	string sorting_strategy_out = (inst.PARAM_APPROACH == "SH") ? inst.PARAM_SORTING_STRATEGY : "none";
	string sorting_sense_out = (inst.PARAM_APPROACH == "SH" && inst.PARAM_SORTING_STRATEGY != "natural")
	                           ? to_string(inst.PARAM_SORTING_SENSE) : "none";

	info_SUMMARY
	<< inst.istname_graph << "\t"
	<< approach_label << "\t"
	<< (inst.PARAM_APPROACH == "SSpooled" ? "pooled" : inst.PARAM_COLORING_METHOD) << "\t"
	<< inst.PARAM_RANDOM_SEED << "\t"
	<< inst.PARAM_TIME_LIMIT << "\t"
	<< sorting_strategy_out << "\t"
	<< sorting_sense_out << "\t"
	<< inst.G->nnodes << "\t"
	<< inst.G->nedges << "\t"
	<< inst.num_colors << "\t";

	if(inst.PARAM_APPROACH == "SH")
	{
		info_SUMMARY
		<< inst.ShimizuBound << "\t"
		<< inst.ShimizuBound_first_policy << "\t"
		<< "Optimal" << "\t"
		<< inst.ShimizuBound_Time << "\t"
		<< "\n";
	}
	else if(inst.PARAM_APPROACH == "SS" || inst.PARAM_APPROACH == "SSpooled")
	{
		info_SUMMARY
		<< inst.SanSegundoBound << "\t"
		<< inst.SanSegundoBound_BestObjVal << "\t"
		<< inst.SanSegundoBound_status << "\t"
		<< inst.SanSegundoBound_Time << "\t"
		<< "\n";
	}
	else if(inst.PARAM_APPROACH == "HFB") 
	{
		info_SUMMARY
		<< inst.HFBBound << "\t"
		<< "none" << "\t"
		<< "Optimal" << "\t"
		<< inst.HFBBound_Time << "\t"
		<< "\n";
	}
	else if(inst.PARAM_APPROACH == "CG") 
	{
		info_SUMMARY
		<< inst.CGBound << "\t"
		<< inst.CGBound_num_cuts << "\t"
		<< (inst.CGBound_TimeLimitHit ? "TimeLimit" : "Optimal") << "\t"
		<< inst.CGBound_Time << "\t"
		<< "\n";
	}
	else if(inst.PARAM_APPROACH == "F11" || inst.PARAM_APPROACH == "F1") 
	{
		info_SUMMARY
		<< inst.F11Bound << "\t"
		<< "none" << "\t"
		<< (inst.F11Bound_TimeLimitHit ? "TimeLimit" : "Optimal") << "\t"
		<< inst.F11Bound_Time << "\t"
		<< "\n";
	}
	
	
	info_SUMMARY.close();


	delete []  inst.istname_graph;
	delete []  inst.istname_weights;

	delete inst.G;


	return 0;
}

