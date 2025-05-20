#include "SanSegundoBound.h"

///////////////////////////////////////////////////////////////////////////////
#define DISABLE_CPLEX_OUTPUT
// #define print_SANSEGUNDO_BOUND_model
// #define save_best_policy
///////////////////////////////////////////////////////////////////////////////

#define MULT 100 //used for scaling

/***************************************************************************/
void SanSegundoBound_ModelBuild(instance *inst)
/***************************************************************************/
{		

	inst->model = IloModel(inst->env);

	// RHO VARIABLES rho[e][u] = fraction of weight of edge e assigned to node u
	inst->rho = IloArray<IloNumVarArray>(inst->env, inst->G->m);
    
	for (int e=0; e < inst->G->m; e++) {
        inst->rho[e] = IloNumVarArray(inst->env, 2, 0, IloInfinity, ILOFLOAT);  
    }

    // PI VARIABLES pi[h] = weight of the heaviest node of color h
    inst->pi = IloNumVarArray (inst->env, inst->num_colors, 0, IloInfinity, ILOFLOAT);


    // OBJECTIVE FUNCTION
    IloExpr obj(inst->env);
    for (int h = 0; h < inst->num_colors; h++) {
        obj += inst->pi[h];
    }
    inst->model.add(IloMinimize(inst->env, obj));
    obj.end();  



	// CONSTRAINTS 
	// 1. rho[e][u] + rho[e][v] >= c_e for all e in E

	for (int e = 0; e < inst->G->m; e++) {
		IloExpr expr(inst->env);
		expr += inst->rho[e][0] + inst->rho[e][1];
		inst->model.add(expr >= inst->G->P[e]);
		expr.end();
	}



	// 2. pi[I] >= sum_{e in delta(u)}(rho[e][u]) for all u in V, I = color(u)

	for (int u = 0; u < inst->G->n; u++)
	{
		IloExpr expr(inst->env);
		for (int  k = inst->G->NFS[u]; k < inst->G->NFS[u+1]; k++)
		{
			expr += inst->rho[inst->G->AFS[k]][0];
		}
	
		for (int  k = inst->G->NBS[u]; k < inst->G->NBS[u+1]; k++ )
		{
			expr += inst->rho[inst->G->ABS[k]][1];
		}
		inst->model.add(expr <= inst->pi[inst->v_color[u]]);
		expr.end();
	}
	


	///////////////////////////////////////// WRITE MODEL /////////////////////////////////////////


#ifdef print_SANSEGUNDO_BOUND_model
	// Write the model to a .lp file
	inst->model.exportModel("SANSEGUNDO_BOUND_model.lp", NULL);
#endif

}




/***************************************************************************/
void SanSegundoBound_ModelSolve(instance *inst)
/***************************************************************************/
{

	inst->cplex = IloCplex(inst->model);
	//////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef DISABLE_CPLEX_OUTPUT
	inst->cplex.setOut(inst->env.getNullStream());
#endif

	inst->cplex.setParam(IloCplex::Param::TimeLimit, inst->PARAM_TIME_LIMIT);
	inst->cplex.setParam(IloCplex::Param::Threads, 1);



	//////////////////////////////////////////////////////////////////////////////////////////////////

	// Set CPLEX LP algorithm to the chosen one
		
	// inst->cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_PRIMAL);
	// inst->cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_DUAL);
	// inst->cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_NET);

	inst->cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_BARRIER);
	inst->cplex.setParam(IloCplex::Param::SolutionType, 2);
	
	// inst->cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_SIFTING);
	// inst->cplex.setParam(IloCplex::Param::RootAlgorithm, CPX_ALG_CONCURRENT);

	



	//////////////////////////////////////////////////////////////////////////////////////////////////
	clock_t time_start=clock();

	inst->cplex.solve();

	clock_t time_end=clock();
	
	inst-> SanSegundoBound_Time=(double)(time_end-time_start)/(double)CLOCKS_PER_SEC;

	
	// Get the optimal solution value
	inst->SanSegundoBound = inst->cplex.getObjValue();
	inst->SanSegundoBound_BestObjVal = inst->cplex.getBestObjValue();
	inst -> SanSegundoBound_status = inst->cplex.getStatus();


	// Print status for debugging
	// cout << "\n\nStatus: " << inst->cplex.getStatus() << endl;
	
	cout << "SanSegundo Bound value: " << inst->SanSegundoBound << endl;



#ifdef save_best_policy
	ofstream sol_policy_SUMMARY(std::string(inst->istname_graph) + ".rho.sol");

	// Print the variables rho[e][u] to the file
	for (int e = 0; e < inst->G->m; e++)
	{
		sol_policy_SUMMARY << inst->G->T[e] + 1 << "\t"
				<< inst->G->H[e] + 1 << "\t"
				<< inst->cplex.getValue(inst->rho[e][0]) << "\t"
				<< inst->cplex.getValue(inst->rho[e][1]) << "\n";
	}

	sol_policy_SUMMARY.close();
#endif


}

/***************************************************************************/
void SanSegundoBound_Free (instance *inst)
/***************************************************************************/
{
	for(int e = 0; e < inst->G->m; e++) {
        inst->rho[e].end();
    }
    inst->rho.end();

    inst->pi.end();

    inst->cplex.end();
	inst->model.end();


    inst->env.end();


}

	
