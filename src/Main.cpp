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
#include "HFBBound.h"


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

	inst.PARAM_APPROACH = new char[10];

	if (argc == 2)
	{
		string arg = argv[1];
		if(arg == "--help")
		{
			cout << "--------------------------------------\n";
			cout << "          PROGRAM PARAMETERS          \n";
			cout << "--------------------------------------\n";
			cout << "Graph instance file" << "\n";
			cout << "Weights file" << "\n";
			cout << "Bounding approach (SS, SH, or HFB)" << "\n";
			cout << "Coloring method (dsatur or random)" << "\n";
			cout << "Random seed (int)\n";
			cout << "Time Limit (seconds)\n";
			cout << "--------------------------------------\n";

			exit(0);
		} 
	}

	else if (argc == 7)
	{
		strcpy(inst.istname_graph, argv[1]);
		strcpy(inst.istname_weights, argv[2]);
		inst.PARAM_APPROACH = argv[3];
		inst.PARAM_COLORING_METHOD = argv[4];
		inst.PARAM_RANDOM_SEED = atoi(argv[5]);
		inst.PARAM_TIME_LIMIT = atof(argv[6]);
	}
	else {cout << "ERROR NUMBER STANDARD PARAMETERS" << endl;exit(2);}

	cout << "\n";
	cout << "istname_graph: ->\t" <<  inst.istname_graph << endl;
	cout << "istname_weights: ->\t" <<  inst.istname_weights << endl;

	cout << "PARAM_APPROACH: ->\t" <<  inst.PARAM_APPROACH << endl;
	cout << "PARAM_COLORING_METHOD: ->\t" <<  inst.PARAM_COLORING_METHOD << endl;

	cout << "PARAM_RANDOM_SEED: ->\t" <<  inst.PARAM_RANDOM_SEED << endl;
	cout << "PARAM_TIME_LIMIT: ->\t" <<  inst.PARAM_TIME_LIMIT << endl;


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

	
	if(inst.PARAM_APPROACH == "SH" || inst.PARAM_APPROACH == "SS" || inst.PARAM_APPROACH == "HFB")
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
		
		cout << "\n\nDONE!!";
		cout << "\n************************************\n\n\n";
	}
	
	else
	{
		cout << "UNRECOGNIZED BOUNDING APPROACH\n";
		exit(2);
	}


	ofstream info_SUMMARY("results.txt", ios::app);
	info_SUMMARY
	<< inst.istname_graph << "\t"
	<< inst.istname_weights << "\t"
	
	<< inst.PARAM_APPROACH << "\t"
	<< inst.PARAM_COLORING_METHOD << "\t"
	<< inst.PARAM_RANDOM_SEED << "\t"
	<< inst.PARAM_TIME_LIMIT << "\t"
	
	<< inst.G->nnodes << "\t"
	<< inst.G->nedges << "\t";


	if(inst.PARAM_APPROACH == "SH") 
	{
		info_SUMMARY
		<< inst.ShimizuBound << "\t"
		<< inst.ShimizuBound_first_policy << "\t"
		<< "Optimal" << "\t"
		<< inst.ShimizuBound_Time << "\t"
		<< "\n";
	}
	else if(inst.PARAM_APPROACH == "SS") 
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
		<< "Optimal" << "\t"
		<< inst.HFBBound_Time << "\t"
		<< "\n";
	}
	
	
	info_SUMMARY.close();


	delete []  inst.istname_graph;
	delete []  inst.istname_weights;

	delete inst.G;


	return 0;
}

