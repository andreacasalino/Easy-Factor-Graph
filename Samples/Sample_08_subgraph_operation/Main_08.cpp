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

#include <Graphical_model.h>
#include <Trainer.h>
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

void print_marginals(Node::Node_factory& source , const vector<string>& subgraph_vars, const vector<float>& empirical) {

	auto joint_shape = source.Get_joint_marginal_distribution(subgraph_vars);
	size_t d, D = subgraph_vars.size();

	cout << "theoretical    computed\n";
	auto it = joint_shape->Get_Distribution().get_iter();
	size_t k = 0;
	while (it.is_not_at_end()) {
		cout << empirical[k] << "      " << it->Get_val() << "  ->  <";
		for (d = 0; d < D; ++d) cout << " " << it->Get_indeces()[d];
		cout << " >" << endl;
		++it;
		++k;
	}

}

void part_01() {

	float Z;
	float alfa = 0.5f, beta = 1.5f;

//build the chain
	Categoric_var A(2, "A"); Categoric_var B(2, "B"); Categoric_var C(2, "C"); Categoric_var D(2, "D");
	Random_Field G;

	G.Insert(*std::make_unique<Potential_Exp_Shape>(Potential_Shape(vector<Categoric_var*>{ &A, & B }, true), alfa).get());
	G.Insert(*std::make_unique<Potential_Exp_Shape>(Potential_Shape(vector<Categoric_var*>{ &B, & C }, true), beta).get());
	G.Insert(*std::make_unique<Potential_Exp_Shape>(Potential_Shape(vector<Categoric_var*>{ &C, & D }, true)).get());
	
	vector<float> marginal_theoretical;

//build the sub-graph ABC and the corresponding marginal distribution
	Z = 2.f *(1.f + expf(alfa) + expf(beta) + expf(alfa)* expf(beta));
	marginal_theoretical.reserve(8);
	marginal_theoretical.push_back(expf(alfa) * expf(beta) / Z);
	marginal_theoretical.push_back(expf(alfa) / Z);
	marginal_theoretical.push_back(1.f / Z);
	marginal_theoretical.push_back(expf(beta) / Z);
	marginal_theoretical.push_back(expf(beta) / Z);
	marginal_theoretical.push_back(1.f / Z);
	marginal_theoretical.push_back(expf(alfa) / Z);
	marginal_theoretical.push_back(expf(alfa) * expf(beta) / Z);
	print_marginals(G, { "A", "B", "C" }, marginal_theoretical);
	cout << endl << endl;

//build the sub-graph AB and the corresponding marginal distribution
	Z = 2.f * (1.f + expf(alfa));
	marginal_theoretical.clear();
	marginal_theoretical.push_back(expf(alfa) / Z);
	marginal_theoretical.push_back(1.f / Z);
	marginal_theoretical.push_back(1.f / Z);
	marginal_theoretical.push_back(expf(alfa) / Z);
	print_marginals(G, { "A", "B" }, marginal_theoretical);
	cout << endl << endl;

}

void part_02() {

//build the graph described in 'Sample 08: Sub-graphing'
	Random_Field graph(Get_prefix() + "Sample_08_graphs/graph.xml");
// set the evidences
	graph.Set_Evidences(vector<pair<string, size_t>>{ {"X1", 0}, { "X2",0 }});

// produce a list of samples for the hidden variables, conditioned by the observed values for the other ones
	list<vector<size_t>> sample = graph.Gibbs_Sampling_on_Hidden_set(500, 200);

	{
		// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
		auto Marginal_A_1234 = graph.Get_joint_marginal_distribution({ "A1" , "A2" ,"A3" ,"A4" });
		vector<Categoric_var*> Var_A1234 = { graph.Find_Variable("A1"), graph.Find_Variable("A2"), graph.Find_Variable("A3"), graph.Find_Variable("A4") };
		list<vector<size_t>> comb_raw = { {0,0,0,0}, {1,1,0,0} };
		Discrete_Distribution::const_Full_Match_finder finder(Marginal_A_1234->Get_Distribution());

		cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=0,X2=0) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.front(), Var_A1234, graph.Get_Actual_Hidden_Set());
		cout << "  computed  " << finder(comb_raw.front())->Get_val() << endl;

		cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=0,X2=0) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.back(), Var_A1234, graph.Get_Actual_Hidden_Set());
		cout << "  computed  " << finder(comb_raw.back())->Get_val() << endl << endl;
	}

	{
		// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
		auto Marginal_B_123 = graph.Get_joint_marginal_distribution({ "B1" , "B2" ,"B3" });
		vector<Categoric_var*> Var_B123 = { graph.Find_Variable("B1"), graph.Find_Variable("B2"), graph.Find_Variable("B3") };
		list<vector<size_t>> comb_raw = { {0,0,0}, {1,1,0} };
		Discrete_Distribution::const_Full_Match_finder finder(Marginal_B_123->Get_Distribution());

		cout << "Prob(B1=0, B2=0, B3=0 | X1=0,X2=0) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.front(), Var_B123, graph.Get_Actual_Hidden_Set());
		cout << "  computed  " << finder(comb_raw.front())->Get_val() << endl;

		cout << "Prob(B1=1, B2=1, B3=0 | X1=0,X2=0) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.back(), Var_B123, graph.Get_Actual_Hidden_Set());
		cout << "  computed  " << finder(comb_raw.back())->Get_val() << endl;
	}

// set different observation values
	graph.Set_Evidences(vector<size_t>{ 1,1 });
	// produce a list of samples for the hidden variables, conditioned by the novel observations
	sample = graph.Gibbs_Sampling_on_Hidden_set(500, 200);

	{
		// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
		auto Marginal_A_1234 = graph.Get_joint_marginal_distribution({ "A1" , "A2" ,"A3" ,"A4" });
		vector<Categoric_var*> Var_A1234 = { graph.Find_Variable("A1"), graph.Find_Variable("A2"), graph.Find_Variable("A3"), graph.Find_Variable("A4") };
		list<vector<size_t>> comb_raw = { {0,0,0,0}, {1,1,0,0} };
		Discrete_Distribution::const_Full_Match_finder finder(Marginal_A_1234->Get_Distribution());

		cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=1,X2=1) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.front(), Var_A1234, graph.Get_Actual_Hidden_Set());
		cout << "  computed  " << finder(comb_raw.front())->Get_val() << endl;

		cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=1,X2=1) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.back(), Var_A1234, graph.Get_Actual_Hidden_Set());
		cout << "  computed  " << finder(comb_raw.back())->Get_val() << endl << endl;
	}

	cout << endl;

}
