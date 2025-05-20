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
			cout << "Bounding approach (SS or SH)" << "\n";
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

	int *heads=NULL;
	int *tails=NULL;
	double *weights_arcs=NULL;
	double *weights_nodes=NULL;

	heads=new int[100000000];
	tails=new int[100000000];
	weights_arcs=new double[100000000];
	weights_nodes=new double[100000000];


	ReadDIMACSFile(inst.istname_graph,&nodes,&edges,tails,heads,false);

	for(int i=0;i<edges;i++){weights_arcs[i]=0.0;}
	for(int i=0;i<nodes;i++){weights_nodes[i]=0.0;}

	cout << "GRAPH BUILDING\n";
	inst.G = buildGraphFF(nodes,edges,heads,tails,weights_nodes,weights_arcs,1);
	cout << "DONE\n";



#ifdef print_ist_features
	printGRAPH(G);
	printFS(G);
	printBS(G);
	printAM(G);
	cin.get();
#endif

	delete[] heads;
	delete[] tails;
	delete []weights_arcs;
	delete []weights_nodes;

	//	//////////////////////////////////////
	//	write_edge_weights_file(&inst);
	//	exit(-1);
	//	//////////////////////////////////////
	//
	//	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//	ofstream info_SUMMARY("info_instance.txt", ios::app);
	//	info_SUMMARY
	//	<< inst.G->n << "\t"
	//	<< inst.G->m << "\t"
	//	<< inst.istname_graph << "\t"
	//	<< "\n";
	//	info_SUMMARY.close();
	//	exit(-1);
	//	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	/////////////////////////////////////////////////
	read_edge_weights(inst.G, inst.istname_weights);
	/////////////////////////////////////////////////

	cout << "\nVertices\t" << inst.G->n << "\tEdges\t" << inst.G->m << endl;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////


	clock_t time_start;
	clock_t time_end;

	
	if(inst.PARAM_APPROACH == "SH" || inst.PARAM_APPROACH == "SS")
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

	
		// for(int i=0;i<inst.G->n;i++)
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
		else
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
	
	<< inst.G->n << "\t"
	<< inst.G->m << "\t";


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
	
	
	info_SUMMARY.close();


	delete []  inst.istname_graph;
	delete []  inst.istname_weights;

	deleteGraphFF(inst.G);


	return 0;
}

