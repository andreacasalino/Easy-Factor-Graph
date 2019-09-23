#include <iostream>
#include <string>
using namespace std;

#include "../../CRF/Header/Graphical_model.h"
#include "../Utilities.h"
using namespace Segugio;
#ifdef _DEBUG
#pragma comment (lib, "../../x64/Debug/CRF.lib")
#else
#pragma comment (lib, "../../x64/Release/CRF.lib")
#endif // DEBUG


void process_sample_1();
void process_sample_2();
void process_sample_3();

int main() {
	///////////////////////////////////////////
	// part 01 graph with a single potential //	
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";
	process_sample_1();
	system("pause");

	///////////////////////////////////////////////////////
	// part 02 graph with two potentials and 3 variables //	
	///////////////////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";
	process_sample_2();
	system("pause");

	////////////////////////////////////////////////////
	// part 03 belief degradation with the chain size //	
	////////////////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 03 \n\n\n";
	cout << "-----------------------\n";
	process_sample_3();



	system("pause");
	return 0;
}

void process_sample_1() {

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

	//make a new belief propagation setting B1=0 as observation
	graph_1.Set_Observation_Set_var({ graph_1.Find_Variable("B") });
	graph_1.Set_Observation_Set_val({ 0 });

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("A"));
	cout << "P(A|B=0)" << endl;
	cout << "theoretical " << endl;
	print_distribution(list<float>({ expf(teta) / Z, 1.f / Z }));
	print_distribution(marginals);
	cout << endl << endl;

	//make a new belief propagation setting B1=1 as observation
	graph_1.Set_Observation_Set_val({ 1 });

	graph_1.Get_marginal_distribution(&marginals, graph_1.Find_Variable("A"));
	cout << "P(A|B=1)" << endl;
	cout << "theoretical " << endl;
	print_distribution(list<float>({ 1.f / Z, expf(teta) / Z }));
	print_distribution(marginals);
	cout << endl << endl;

}

void process_sample_2() {

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
	graph_2.Set_Observation_Set_var({ graph_2.Find_Variable("C") });
	graph_2.Set_Observation_Set_val({ 1 });

	float Z = 1.f + expf(alfa) + expf(beta) + expf(alfa)*expf(beta);

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
	graph_2.Set_Observation_Set_var({ graph_2.Find_Variable("B") });
	graph_2.Set_Observation_Set_val({ 1 });

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

	if (chain_size < 2) {
		system("ECHO invalid chain size");
		abort();
	}

	list<Categoric_var*> Y;
	for (size_t k = 0; k < chain_size; k++)
		Y.push_back(new Categoric_var(var_size, "Y_" + to_string(k)));
	auto it1 = Y.begin();
	auto it2 = it1; it2++;

	Graph graph;

	while(it2 != Y.end()) {
		Potential_Exp_Shape temp(new Potential_Shape({ *it1, *it2 }, true), w);
		graph.Insert(&temp);
		it1++; it2++;
	}

	graph.Set_Observation_Set_var({ graph.Find_Variable(Y.front()->Get_name())  });
	graph.Set_Observation_Set_val({ 0 });
	list<float> prob;
	graph.Get_marginal_distribution(&prob, graph.Find_Variable(Y.back()->Get_name()));

	print_distribution(prob);
	cout << endl;

	for (auto it = Y.begin(); it != Y.end(); it++)
		delete *it;

}
void process_sample_3() {

	for (int k = 2; k <= 10; k++) {
		cout << "size " << k << ":   ";
		process_chain(k, 5, 3.5f);
	}

}