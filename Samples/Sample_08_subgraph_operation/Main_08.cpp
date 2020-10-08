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

#include <model/RandomField.h>
#include <distribution/FullMatchFinder.h>
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

void print_marginals(node::Node::NodeFactory& source , const vector<string>& subgraph_vars, const vector<float>& empirical) {

	pot::Factor joint_shape = source.GetJointMarginalDistribution(subgraph_vars);
	size_t d, D = subgraph_vars.size();

	cout << "theoretical    computed\n";
	size_t k = 0;
	distr::DiscreteDistribution::constIterator it = joint_shape.GetDistribution().getIter();
	itr::forEach<distr::DiscreteDistribution::constIterator>(it, [&empirical, &k, &d, &D](distr::DiscreteDistribution::constIterator& itt) {
		cout << empirical[k] << "      " << itt->GetVal() << "  ->  <";
		for (d = 0; d < D; ++d) cout << " " << itt->GetIndeces()[d];
		cout << " >" << endl;
		++k;
	});

}

void part_01() {

	float Z;
	float alfa = 0.5f, beta = 1.5f;

//build the chain
	CategoricVariable A(2, "A"); CategoricVariable B(2, "B"); CategoricVariable C(2, "C"); CategoricVariable D(2, "D");
	model::RandomField G;

	G.InsertMove(pot::ExpFactor(pot::Factor(vector<CategoricVariable*>{ &A, & B }, true), alfa));
	G.InsertMove(pot::ExpFactor(pot::Factor(vector<CategoricVariable*>{ &B, & C }, true), beta));
	G.InsertMove(pot::ExpFactor(pot::Factor(vector<CategoricVariable*>{ &C, & D }, true)));
	
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
	model::RandomField graph(Get_prefix() + "Sample_08_graphs/graph.xml");
// set the evidences
	graph.SetEvidences(vector<pair<string, size_t>>{ {"X1", 0}, { "X2",0 }});

// produce a list of samples for the hidden variables, conditioned by the observed values for the other ones
	list<vector<size_t>> sample = graph.GibbsSamplingHiddenSet(500, 200);

	{
		// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
		pot::Factor Marginal_A_1234 = graph.GetJointMarginalDistribution({ "A1" , "A2" ,"A3" ,"A4" });
		vector<CategoricVariable*> Var_A1234 = { graph.FindVariable("A1"), graph.FindVariable("A2"), graph.FindVariable("A3"), graph.FindVariable("A4") };
		list<vector<size_t>> comb_raw = { {0,0,0,0}, {1,1,0,0} };
		distr::DiscreteDistribution::constFullMatchFinder finder(Marginal_A_1234.GetDistribution());

		cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=0,X2=0) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.front(), Var_A1234, graph.GetHiddenSet());
		cout << "  computed  " << finder(comb_raw.front())->GetVal() << endl;

		cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=0,X2=0) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.back(), Var_A1234, graph.GetHiddenSet());
		cout << "  computed  " << finder(comb_raw.back())->GetVal() << endl << endl;
	}

	{
		// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
		pot::Factor Marginal_B_123 = graph.GetJointMarginalDistribution({ "B1" , "B2" ,"B3" });
		vector<CategoricVariable*> Var_B123 = { graph.FindVariable("B1"), graph.FindVariable("B2"), graph.FindVariable("B3") };
		list<vector<size_t>> comb_raw = { {0,0,0}, {1,1,0} };
		distr::DiscreteDistribution::constFullMatchFinder finder(Marginal_B_123.GetDistribution());

		cout << "Prob(B1=0, B2=0, B3=0 | X1=0,X2=0) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.front(), Var_B123, graph.GetHiddenSet());
		cout << "  computed  " << finder(comb_raw.front())->GetVal() << endl;

		cout << "Prob(B1=1, B2=1, B3=0 | X1=0,X2=0) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.back(), Var_B123, graph.GetHiddenSet());
		cout << "  computed  " << finder(comb_raw.back())->GetVal() << endl;
	}

// set different observation values
	graph.SetEvidences(vector<size_t>{ 1,1 });
	// produce a list of samples for the hidden variables, conditioned by the novel observations
	sample = graph.GibbsSamplingHiddenSet(500, 200);

	{
		// compute the marginal probabilities of the following two combinations (values refer to variables in the subgraph, i.e. A1, 2, 3, 4)
		pot::Factor Marginal_A_1234 = graph.GetJointMarginalDistribution({ "A1" , "A2" ,"A3" ,"A4" });
		vector<CategoricVariable*> Var_A1234 = { graph.FindVariable("A1"), graph.FindVariable("A2"), graph.FindVariable("A3"), graph.FindVariable("A4") };
		list<vector<size_t>> comb_raw = { {0,0,0,0}, {1,1,0,0} };
		distr::DiscreteDistribution::constFullMatchFinder finder(Marginal_A_1234.GetDistribution());

		cout << "Prob(A1=0, A2=0, A3=0,A4=0 | X1=1,X2=1) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.front(), Var_A1234, graph.GetHiddenSet());
		cout << "  computed  " << finder(comb_raw.front())->GetVal() << endl;

		cout << "Prob(A1=1, A2=1, A3=0,A4=0 | X1=1,X2=1) empirical   ";
		cout << Get_empirical_frequencies(sample, comb_raw.back(), Var_A1234, graph.GetHiddenSet());
		cout << "  computed  " << finder(comb_raw.back())->GetVal() << endl << endl;
	}

	cout << endl;

}
