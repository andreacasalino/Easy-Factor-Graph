/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 02: Belief propagation, part A' of the documentation

#include <iostream>
#include <string>
using namespace std;

#include "../../EFG/Header/Graphical_model.h"
#include "../Utilities.h"
using namespace EFG;


void part_01();
void part_02();
void part_03();

int main() {

	///////////////////////////////////////////
	// part 01 graph with a single potential //	
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";
	part_01();

	///////////////////////////////////////////////////////
	// part 02 graph with two potentials and 3 variables //	
	///////////////////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";
	part_02();

	////////////////////////////////////////////////////
	// part 03 belief degradation with the chain size //	
	////////////////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 03 \n\n\n";
	cout << "-----------------------\n";
	part_03();

	return 0;
}

void part_01() {

	//create a simple graph with two nodes
	Categoric_var A1(2, "A");
	Categoric_var B1(2, "B");

	Potential_Shape* shape1 = new  Potential_Shape({ &A1, &B1 });
	shape1->Add_value({ 1,1 }, 1.f);
	shape1->Add_value({ 0,0 }, 1.f);

	float teta = 2.f;
	float Z = 1.f + expf(teta);

	Potential_Exp_Shape Psi1(shape1, teta);

	Graph graph_1;
	graph_1.Insert(&Psi1);
	list<float> marginals;

	//make a new belief propagation setting B=0 as observation
	graph_1.Set_Evidences({ graph_1.Find_Variable("B") }, { 0 });

	//compute the marginal probability of A s.t. the evidence of B: values are compared with the theoretical result (see 'Sample 02: Belief propagation, part A / part 01')
	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("A"));
	cout << "P(A|B=0)" << endl;
	cout << "theoretical " << endl;
	print_distribution(list<float>({ expf(teta) / Z, 1.f / Z }));
	print_distribution(marginals);
	cout << endl << endl;

	//make a new belief propagation setting B1=1 as observation
	graph_1.Set_Evidences( { 1 });

	//compute the new marginals
	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("A"));
	cout << "P(A|B=1)" << endl;
	cout << "theoretical " << endl;
	print_distribution(list<float>({ 1.f / Z, expf(teta) / Z }));
	print_distribution(marginals);
	cout << endl << endl;

}

void part_02() {

	//build the graph described in 'Sample 02: Belief propagation, part A / part 02'
	Categoric_var A2(2, "A");
	Categoric_var B2(2, "B");
	Categoric_var C2(2, "C");

	float alfa = 0.5f, beta = 1.f;

	Potential_Shape* shape_BC = new Potential_Shape({ &B2, &C2 });
	shape_BC->Add_value({ 0,0 }, 1.f);
	shape_BC->Add_value({ 1,1 }, 1.f);
	Potential_Exp_Shape Psi_BC(shape_BC, alfa);

	Potential_Shape* shape_AB = new Potential_Shape({ &A2, &B2 });
	shape_AB->Add_value({ 0,0 }, 1.f);
	shape_AB->Add_value({ 1,1 }, 1.f);
	Potential_Exp_Shape Psi_AB(shape_AB, beta);

	Graph graph_2;
	graph_2.Insert(&Psi_AB);
	graph_2.Insert(&Psi_BC);

	//make a new belief propagation setting C2=1 as observation
	graph_2.Set_Evidences({ graph_2.Find_Variable("C") }, { 1 });

	float Z = 1.f + expf(alfa) + expf(beta) + expf(alfa)*expf(beta);

	//compute the marginals of A,B and then compare results with the theoretial ones 
	//(see also Sample 2/ part 2 of the documentation)
	list<float> marginals;
	graph_2.Get_marginal_distribution(&marginals, graph_2.Find_Variable("B"));
	cout << "P(B|C=1)\n";
	cout << "theoretical " << endl;
	print_distribution(list<float>({ (1.f + expf(beta)) / Z, expf(alfa)*(1.f + expf(beta)) / Z }));
	print_distribution(marginals);

	graph_2.Get_marginal_distribution(&marginals, graph_2.Find_Variable("A"));
	cout << "P(A|C=1)\n";
	cout << "theoretical " << endl;
	print_distribution(list<float>({ (expf(alfa) + expf(beta)) / Z, (1.f + expf(alfa) * expf(beta)) / Z }));
	print_distribution(marginals);


	//make a new belief propagation setting B2=1 as observation
	graph_2.Set_Evidences({ graph_2.Find_Variable("B") }, { 1 });

	Z = 1.f + expf(beta);

	graph_2.Get_marginal_distribution(&marginals, graph_2.Find_Variable("A"));
	cout << "P(A|B=1)\n";
	cout << "theoretical " << endl;
	print_distribution(list<float>({ 1.f / Z, expf(beta) / Z }));
	print_distribution(marginals);

	Z = 1.f + expf(alfa);

	graph_2.Get_marginal_distribution(&marginals, graph_2.Find_Variable("C"));
	cout << "P(C|B=1)\n";
	cout << "theoretical " << endl;
	print_distribution(list<float>({ 1.f / Z, expf(alfa) / Z }));
	print_distribution(marginals);

}

void process_chain(const size_t& chain_size, const size_t& var_size, const float& w) {

	if (chain_size < 2) throw 0; //invalid chain size

	//build the set of variables in the chain
	list<Categoric_var*> Y;
	for (size_t k = 0; k < chain_size; k++)
		Y.push_back(new Categoric_var(var_size, "Y_" + to_string(k)));
	auto it1 = Y.begin();
	auto it2 = it1; it2++;

	Graph graph;

	//build the correlating potentials and add it to the chain
	while(it2 != Y.end()) {
		Potential_Exp_Shape temp(new Potential_Shape({ *it1, *it2 }, true), w);
		graph.Insert(&temp); //potentials are internally copied
		it1++; it2++;
	}

	//set Y_0 as an observations and compute the marginals of the last variable in the chain
	graph.Set_Evidences({ graph.Find_Variable(Y.front()->Get_name()) }, {0});
	list<float> prob;
	graph.Get_marginal_distribution(&prob, graph.Find_Variable(Y.back()->Get_name()));

	print_distribution(prob);
	cout << endl;

	for (auto it = Y.begin(); it != Y.end(); it++)
		delete *it;

}
void part_03() {

	//build chain graphs described in 'Sample 02: Belief propagation, part A / part 03'

	size_t Dom_size = 5;

	for (int k = 2; k <= 10; k++) {
		cout << "chain size equal to " << k << ", marginals of Y_n:   ";
		process_chain(k, Dom_size, 3.5f);
	}

}