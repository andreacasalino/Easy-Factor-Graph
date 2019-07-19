#include <iostream>
#include <string>
#include <vector>
using namespace std;

#include "../../CRF/Header/Graphical_model.h"
#include "../Prefix_path.h"
#include "../Utilities.h"
using namespace Segugio;
#ifdef _DEBUG
#pragma comment (lib, "../../x64/Debug/CRF.lib")
#else
#pragma comment (lib, "../../x64/Release/CRF.lib")
#endif // DEBUG


int main() {
	string prefix = compute_prefix() + "Sample_09_subgraph_operation" + "/";

//build the graph
	Random_Field graph("graph.xml", prefix);
	list<Categoric_var*> Ob_var = { graph.Find_Variable("X1"),graph.Find_Variable("X2") };
	list<float> marginal_probs;


// set the observation values
	graph.Set_Observation_Set_var(Ob_var);
	list<Categoric_var*> Entire_Hidden_set;
	graph.Get_Actual_Hidden_Set(&Entire_Hidden_set);
	graph.Set_Observation_Set_val({ 0,0 });	
// produce a list of samples for the hidden variables, conditioned by the observed values for the other ones
	list<list<size_t>> sample;
	graph.Gibbs_Sampling_on_Hidden_set(&sample, 500, 200);

	list<Categoric_var*> Var_A1234 = { graph.Find_Variable("A1") ,graph.Find_Variable("A2") ,graph.Find_Variable("A3") ,graph.Find_Variable("A4") };
//build the subgraph involving {A1,2,3,4}
	SubGraph* sub_A_1234 = new SubGraph(&graph, Var_A1234);
// compute the marginal probabilities of the follwoing two combinations (values refer to varaibles in the subgraph, i.e. A1, 2, 3, 4)
	list<list<size_t>> sub_A_1234_combinations = { {0,0,0,0}, {1,1,0,0} };
	sub_A_1234->Get_marginal_prob_combinations(&marginal_probs, sub_A_1234_combinations, Var_A1234);

	cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=0,X2=0) empirical   ";
	cout << Get_empirical_frequencies(sample, sub_A_1234_combinations.front(), Var_A1234, Entire_Hidden_set) ;
	cout << "  computed  " << marginal_probs.front() << endl;

	cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=0,X2=0) empirical   ";
	cout << Get_empirical_frequencies(sample, sub_A_1234_combinations.back(), Var_A1234, Entire_Hidden_set) ;
	cout << "  computed  " << marginal_probs.back() << endl << endl;

//build the subgraph involving {B1,2,3}
	list<Categoric_var*> Var_B123 = { graph.Find_Variable("B1") ,graph.Find_Variable("B2") ,graph.Find_Variable("B3") };
	SubGraph* sub_B_123 = new SubGraph(&graph, Var_B123);
	// compute the marginal probabilities of the follwoing two combinations (values refer to varaibles in the subgraph, i.e. A1, 2, 3, 4)
	list<list<size_t>> sub_B_123_combinations = { {0,0,0}, {1,1,0} };
	sub_B_123->Get_marginal_prob_combinations(&marginal_probs, sub_B_123_combinations, Var_B123);

	cout << "Prob(B1=0, B2=0, B3=0 | X1=0,X2=0) empirical   ";
	cout << Get_empirical_frequencies(sample, sub_B_123_combinations.front(), Var_B123, Entire_Hidden_set) ;
	cout << "  computed  " << marginal_probs.front() << endl;

	cout << "Prob(B1=1, B2=1, B3=0 | X1=0,X2=0) empirical   ";
	cout << Get_empirical_frequencies(sample, sub_B_123_combinations.back(), Var_B123, Entire_Hidden_set) ;
	cout << "  computed  " << marginal_probs.back() << endl << endl;



	system("pause");

// set different observation values
	graph.Set_Observation_Set_val({ 1,1 });
	// produce a list of samples for the hidden variables, conditioned by the novel observations
	graph.Gibbs_Sampling_on_Hidden_set(&sample, 500, 200);

// the previous subgraph is not affected by the above modification, therefore recompute the subgraph to consider these observations
	delete sub_A_1234;
	sub_A_1234 = new SubGraph(&graph, Var_A1234);
	sub_A_1234_combinations = { {0,0,0,0}, {1,1,0,0} };
	sub_A_1234->Get_marginal_prob_combinations(&marginal_probs, sub_A_1234_combinations, Var_A1234);

	cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=1,X2=1) empirical   ";
	cout << Get_empirical_frequencies(sample, sub_A_1234_combinations.front(), Var_A1234, Entire_Hidden_set) ;
	cout << "  computed  " << marginal_probs.front() << endl;

	cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=1,X2=1) empirical   ";
	cout << Get_empirical_frequencies(sample, sub_A_1234_combinations.back(), Var_A1234, Entire_Hidden_set) ;
	cout << "  computed  " << marginal_probs.back() << endl << endl;


	delete sub_B_123;
	delete sub_A_1234;

	system("pause");
	return 0;
}