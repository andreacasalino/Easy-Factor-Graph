/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

 // This example is also explained in the documentation (the .pdf file), Section 'Sample 08: Sub-graphing'


#include <iostream>
#include <string>
using namespace std;

#include "../../EFG/Header/Graphical_model.h"
#include "../../EFG/Header/Subgraph.h"
#include "../../EFG/Header/Trainer.h"
#include "../Utilities.h"
using namespace EFG;

void part_01();
void part_02();

int main() {

	///////////////////////////////////////////
	//            part 01 graph_2            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";
	part_01();

	///////////////////////////////////////////
	//            part 02 graph_3, tunable and non-tunable weights           //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";
	part_02();

	return 0;
}


void print_computed_theoretical(Potential_Shape* computed, const vector<float>& theoretical) {

	vector<vector<size_t>> dom;
	vector<float> img;
	computed->Get_domain(&dom);
	computed->Get_images(&img);

	size_t k, K = img.size();
	size_t s, S = dom.front().size();
	cout << "computed theoretical combination\n";
	for (k = 0; k < K; k++) {
		cout << img[k] << " " << theoretical[k];
		for (s = 0; s < S; s++)
			cout << " " << dom[k][s];
		cout << endl;
	}

}
void part_01() {

//build the chain
	Categoric_var A(2, "A"); Categoric_var B(2, "B"); Categoric_var C(2, "C"); Categoric_var D(2, "D");
	vector<Potential_Exp_Shape> Pot;
	Pot.reserve(4);
	Pot.emplace_back(*(new Potential_Shape({ &A, &B }, true)));
	Pot.emplace_back(*(new Potential_Shape({ &B, &C }, true)));
	Pot.emplace_back(*(new Potential_Shape({ &C, &D }, true)));
	Random_Field G(false);
	for (size_t k = 0; k < Pot.size(); k++)
		G.Insert(Pot[k]);

	vector<float> marginal_theoretical;
	float Z;
	float alfa = 0.5f, beta = 1.5f;
	G.Set_tunable({ alfa, beta, 1.f });

//build the sub-graph ABC and the correspinding marginal distribution
	SubGraph SG1(G, { &A, &B, &C });
	auto Marginal = SG1.Get_joint_marginal_distribution();
	Z = 2.f *(1.f + expf(alfa) + expf(beta) + expf(alfa)* expf(beta));
	marginal_theoretical.reserve(8);
	marginal_theoretical.push_back(expf(alfa) * expf(beta) / Z);
	marginal_theoretical.push_back( expf(beta) / Z);
	marginal_theoretical.push_back(1.f / Z);
	marginal_theoretical.push_back(expf(alfa)  / Z);
	marginal_theoretical.push_back(expf(alfa)  / Z);
	marginal_theoretical.push_back(1.f / Z);
	marginal_theoretical.push_back( expf(beta) / Z);
	marginal_theoretical.push_back(expf(alfa) * expf(beta) / Z);
	print_computed_theoretical(Marginal.get(), marginal_theoretical);
	cout << endl << endl;

//build the sub-graph AB and the correspinding marginal distribution
	SubGraph SG2(SG1, { &A, &B });
	Marginal = SG2.Get_joint_marginal_distribution();
	Z = 2.f * (1.f + expf(alfa));
	marginal_theoretical.clear();
	marginal_theoretical.push_back(expf(alfa) / Z);
	marginal_theoretical.push_back(1.f / Z);
	marginal_theoretical.push_back(1.f / Z);
	marginal_theoretical.push_back(expf(alfa) / Z);
	print_computed_theoretical(Marginal.get(), marginal_theoretical);
	cout << endl << endl;

}

void part_02() {

//build the graph described in 'Sample 08: Sub-graphing'
	Random_Field graph(string("./Sample_08_graphs/graph.xml"));
	list<Categoric_var*> Ob_var = { graph.Find_Variable("X1"),graph.Find_Variable("X2") };
	vector<const I_Potential::I_Distribution_value*> values;

// set the evidences
	graph.Set_Evidences(Ob_var, { 0,0 });
	list<Categoric_var*> Entire_Hidden_set;
	graph.Get_Actual_Hidden_Set(&Entire_Hidden_set);

// produce a list of samples for the hidden variables, conditioned by the observed values for the other ones
	list<list<size_t>> sample;
	graph.Gibbs_Sampling_on_Hidden_set(&sample, 500, 200);

//build the subgraph involving {A1,2,3,4}
	list<Categoric_var*> Var_A1234 = { graph.Find_Variable("A1") ,graph.Find_Variable("A2") ,graph.Find_Variable("A3") ,graph.Find_Variable("A4") };
	SubGraph sub_A_1234(graph, Var_A1234);
// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
	auto Marginal_A_1234 = sub_A_1234.Get_joint_marginal_distribution();
	list<list<size_t>> comb_raw1 = { {0,0,0,0}, {1,1,0,0} };
	I_Potential::combinations sub_A_1234_combinations(comb_raw1, Var_A1234);
	sub_A_1234_combinations.Find_images_single_matches(&values, *Marginal_A_1234);

	cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=0,X2=0) empirical   ";
	cout << Get_empirical_frequencies(sample, comb_raw1.front(), Var_A1234, Entire_Hidden_set) ;
	cout << "  computed  " << values.front()->Get_val() << endl;

	cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=0,X2=0) empirical   ";
	cout << Get_empirical_frequencies(sample, comb_raw1.back(), Var_A1234, Entire_Hidden_set) ;
	cout << "  computed  " << values.back()->Get_val() << endl << endl;

//build the subgraph involving {B1,2,3}
	list<Categoric_var*> Var_B123 = { graph.Find_Variable("B1") ,graph.Find_Variable("B2") ,graph.Find_Variable("B3") };
	SubGraph sub_B_123(graph, Var_B123);
	// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
	auto Marginal_B_123 = sub_B_123.Get_joint_marginal_distribution();
	list<list<size_t>> comb_raw2 = { {0,0,0}, {1,1,0} };
	I_Potential::combinations sub_B_123_combinations(comb_raw2, Var_B123);
	sub_B_123_combinations.Find_images_single_matches(&values, *Marginal_B_123);

	cout << "Prob(B1=0, B2=0, B3=0 | X1=0,X2=0) empirical   ";
	cout << Get_empirical_frequencies(sample, comb_raw2.front(), Var_B123, Entire_Hidden_set) ;
	cout << "  computed  " << values.front()->Get_val() << endl;

	cout << "Prob(B1=1, B2=1, B3=0 | X1=0,X2=0) empirical   ";
	cout << Get_empirical_frequencies(sample, comb_raw2.back(), Var_B123, Entire_Hidden_set) ;
	cout << "  computed  " << values.back()->Get_val() << endl << endl;

// set different observation values
	graph.Set_Evidences({ 1,1 });
	// produce a list of samples for the hidden variables, conditioned by the novel observations
	graph.Gibbs_Sampling_on_Hidden_set(&sample, 500, 200);

// recompute the marginals according to the new evidences
	Marginal_A_1234 = sub_A_1234.Get_joint_marginal_distribution();
	sub_A_1234_combinations.Find_images_single_matches(&values, *Marginal_A_1234);

	cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=1,X2=1) empirical   ";
	cout << Get_empirical_frequencies(sample, comb_raw1.front(), Var_A1234, Entire_Hidden_set);
	cout << "  computed  " << values.front()->Get_val() << endl;

	cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=1,X2=1) empirical   ";
	cout << Get_empirical_frequencies(sample, comb_raw1.back(), Var_A1234, Entire_Hidden_set);
	cout << "  computed  " << values.back()->Get_val() << endl << endl;

	cout << endl;

}