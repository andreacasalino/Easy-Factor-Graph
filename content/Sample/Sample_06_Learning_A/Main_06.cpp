#include <iostream>
#include <string>
using namespace std;

#include "../../CRF/Header/Graphical_model.h"
#include "../../CRF/Header/Trainer.h"
#include "../Prefix_path.h"
#include "../Utilities.h"
using namespace Segugio;
#ifdef _DEBUG
#pragma comment (lib, "../../x64/Debug/CRF.lib")
#else
#pragma comment (lib, "../../x64/Release/CRF.lib")
#endif // DEBUG



void part_01(const string& prefix);
void part_02(const string& prefix);
void part_03(const string& prefix);
void part_04(const string& prefix);

int main() {
	string prefix =  compute_prefix() + "Sample_06_Learning_A" + "/";

	///////////////////////////////////////////
	//            part 01 graph_1            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 01 \n\n\n";
	cout << "-----------------------\n";
	part_01(prefix);
	system("pause");

	///////////////////////////////////////////
	//            part 02 graph_2            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 02 \n\n\n";
	cout << "-----------------------\n";
	part_02(prefix);
	system("pause");

	///////////////////////////////////////////
	//            part 03 graph_3            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 03 \n\n\n";
	cout << "-----------------------\n";
	part_03(prefix);

	///////////////////////////////////////////
	//            part 04 graph_4            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 04 \n\n\n";
	cout << "-----------------------\n";
	part_04(prefix);

	system("pause");
	return 0;
}

void part_01(const string& prefix) {

	float w = 1.5f;

	//create a graph with a single binary potential, having w as weight
	Categoric_var A(2, "A");
	Categoric_var B(2, "B");
	Potential_Shape* shape = new Potential_Shape({ &A,&B }, prefix + "Shape.txt");
	Potential_Exp_Shape pot(shape, w);
	Graph graph_1;
	graph_1.Insert(&pot);

	//sample from the created graph
	list<list<size_t>> sample;
	graph_1.Gibbs_Sampling_on_Hidden_set(&sample, 500, 100);

	list<float> marginals;

	Categoric_var* A_in_graph = graph_1.Find_Variable("A");
	Categoric_var* B_in_graph = graph_1.Find_Variable("B");

	//check the correctness of the marginal probabilty distributions
	Get_empirical_frequencies(&marginals, sample, A_in_graph, { A_in_graph,B_in_graph });
	cout << "P(A| any values) (as freq in the sampled set)\n";
	print_distribution(marginals); cout << endl;

	Get_empirical_frequencies(&marginals, sample, B_in_graph, { A_in_graph,B_in_graph });
	cout << "P(B| any values) (as freq in the sampled set)\n";
	print_distribution(marginals); cout << endl;

	//check the correctness of the probability of the combinations

	float Z = 2.f*(1.f + expf(w));

	cout << "freq <0,0> " << Get_empirical_frequencies(sample, { 0,0 }) << " theoretical: " << expf(w) / Z << endl;
	cout << "freq <0,1> " << Get_empirical_frequencies(sample, { 0,1 }) << " theoretical: " << 1.f / Z << endl;

}

void part_02(const string& prefix) {

	//build graph_2
	Categoric_var A(2, "A");
	Categoric_var B(2, "B");
	Categoric_var C(2, "C");

	float alfa = 1.f, beta = 3.f, gamma = 0.1f;

	Potential_Exp_Shape Pot_AB(new Potential_Shape({ &A,&B }, prefix + "Shape.txt"), alfa);
	Potential_Exp_Shape Pot_AC(new Potential_Shape({ &A,&C }, prefix + "Shape.txt"), beta);
	Potential_Exp_Shape Pot_BC(new Potential_Shape({ &B,&C }, prefix + "Shape.txt"), gamma);

	Random_Field graph_2;
	graph_2.Insert(&Pot_AB);
	graph_2.Insert(&Pot_AC);
	graph_2.Insert(&Pot_BC);

	//extract some samples from the graph with a Gibbs sampling method, for building a train set
	list<list<size_t>> samples;
	graph_2.Gibbs_Sampling_on_Hidden_set(&samples, 500, 500);
	Print_set_as_training_set(prefix + "Train_set.txt", { &A,&B,&C }, samples);

	//check empirical frequency for some combinations
	float Z = 2.f*(expf(alfa) + expf(beta) + expf(gamma) + expf(alfa)*expf(beta)*expf(gamma));

	cout << "Gibbs sampler results\n";
	cout << "freq <0,0,0> " << Get_empirical_frequencies(samples, { 0,0,0 }) << " theoretical: " << expf(alfa)*expf(beta)*expf(gamma) / Z << endl;
	cout << "freq <1,0,0> " << Get_empirical_frequencies(samples, { 1,0,0 }) << " theoretical: " << expf(gamma) / Z << endl;
	cout << "freq <0,1,0> " << Get_empirical_frequencies(samples, { 0,1,0 }) << " theoretical: " << expf(beta) / Z << endl;
	cout << "freq <1,1,0> " << Get_empirical_frequencies(samples, { 1,1,0 }) << " theoretical: " << expf(alfa) / Z << endl;



	//build a second graph, with the same potentials, but all weights equal to 1. Then use the previous train set to train 
	//this model, for obtaining a combination of weights similar to the original ones
	Categoric_var A2(2, "A");
	Categoric_var B2(2, "B");
	Categoric_var C2(2, "C");

	Potential_Exp_Shape Pot_AB_to_learn(new Potential_Shape({ &A2,&B2 }, prefix + "Shape.txt"));
	Potential_Exp_Shape Pot_AC_to_learn(new Potential_Shape({ &A2,&C2 }, prefix + "Shape.txt"));
	Potential_Exp_Shape Pot_BC_to_learn(new Potential_Shape({ &B2,&C2 }, prefix + "Shape.txt"));

	Random_Field graph_to_learn;
	graph_to_learn.Insert(&Pot_AB_to_learn);
	graph_to_learn.Insert(&Pot_AC_to_learn);
	graph_to_learn.Insert(&Pot_BC_to_learn);

	//train graph_to_learn with a fixed step gradient descend algorithm
	Training_set Set(prefix + "Train_set.txt");
	auto Learner = I_Trainer::Get_fixed_step(1.f);
	list<float> likelihood_story;
	Learner->Train(&graph_to_learn, &Set, 50, &likelihood_story); 
	delete Learner;

	cout << "\n\n\n evolution of the likelihood of the model during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	list<float> w;
	cout << "\n\n\n real weights of the model\n";
	Graph_Learnable::Weights_Manager::Get_tunable_w(&w, &graph_2);
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	Graph_Learnable::Weights_Manager::Get_tunable_w(&w, &graph_to_learn);
	print_distribution(w); cout << endl;
	cout << endl;


	list<float> marginals;
	graph_2.Set_Observation_Set_var({ graph_2.Find_Variable("C")});
	graph_2.Set_Observation_Set_val({ 0 });
	graph_2.Get_marginal_distribution(&marginals, graph_2.Find_Variable("A"));
	cout << "P(A|C=0) real model    ";
	print_distribution(marginals); cout << endl;

	graph_to_learn.Set_Observation_Set_var({ graph_to_learn.Find_Variable("C") });
	graph_to_learn.Set_Observation_Set_val({ 0 });
	graph_to_learn.Get_marginal_distribution(&marginals, graph_to_learn.Find_Variable("A"));
	cout << "P(A|C=0) learnt model  ";
	print_distribution(marginals); cout << endl;
}

void part_03(const string& prefix) {

	//build graph_2
	Categoric_var A(2, "A");
	Categoric_var B(2, "B");
	Categoric_var C(2, "C");

	float alfa = 3.f, beta = 2.f;

	Potential_Exp_Shape Pot_AB(new Potential_Shape({ &A,&B }, prefix + "Shape.txt"), alfa);
	Potential_Exp_Shape Pot_AC(new Potential_Shape({ &A,&C }, prefix + "Shape.txt"), beta);

	Random_Field graph_2;
	graph_2.Insert(&Pot_AB);
	graph_2.Insert(&Pot_AC);

	//extract some samples from the graph with a Gibbs sampling method, for building a train set
	list<list<size_t>> samples;
	graph_2.Gibbs_Sampling_on_Hidden_set(&samples, 500, 500);
	Print_set_as_training_set(prefix + "Train_set.txt", { &A,&B,&C }, samples);

	//build a second graph, with the same potentials, but all weights equal to 1. Then use the previous train set to train 
	//this model, for obtaining a combination of weights similar to the original ones
	Categoric_var A2(2, "A");
	Categoric_var B2(2, "B");
	Categoric_var C2(2, "C");

	Potential_Exp_Shape Pot_AB_to_learn(new Potential_Shape({ &A2,&B2 }, prefix + "Shape.txt"), alfa);
	Potential_Exp_Shape Pot_AC_to_learn(new Potential_Shape({ &A2,&C2 }, prefix + "Shape.txt"));

	Random_Field graph_to_learn;
	graph_to_learn.Insert(&Pot_AB_to_learn, false); // the weight of this potential will be constant
	graph_to_learn.Insert(&Pot_AC_to_learn);

	//train graph_to_learn with a fixed step gradient descend algorithm
	Training_set Set(prefix + "Train_set.txt");
	auto Learner = I_Trainer::Get_fixed_step(1.f);
	list<float> likelihood_story;
	Learner->Train(&graph_to_learn, &Set, 50, &likelihood_story);
	delete Learner;

	cout << "\n\n\n evolution of the likelihood of the model during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	list<float> w;
	cout << "\n\n\n real weights of the model\n";
	Graph_Learnable::Weights_Manager::Get_tunable_w(&w, &graph_2);
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	Graph_Learnable::Weights_Manager::Get_tunable_w(&w, &graph_to_learn);
	print_distribution(w); cout << endl;
	cout << endl;


	list<float> marginals;
	graph_2.Set_Observation_Set_var({ graph_2.Find_Variable("C") });
	graph_2.Set_Observation_Set_val({ 0 });
	graph_2.Get_marginal_distribution(&marginals, graph_2.Find_Variable("A"));
	cout << "P(A|C=0) real model    ";
	print_distribution(marginals); cout << endl;

	graph_to_learn.Set_Observation_Set_var({ graph_to_learn.Find_Variable("C") });
	graph_to_learn.Set_Observation_Set_val({ 0 });
	graph_to_learn.Get_marginal_distribution(&marginals, graph_to_learn.Find_Variable("A"));
	cout << "P(A|C=0) learnt model  ";
	print_distribution(marginals); cout << endl;
}

void part_04(const string& prefix) {

	//build graph_3
	Random_Field graph_3("graph_3.xml", prefix);

	//extract some samples from the graph with a Gibbs sampling method, for building a train set
	list<list<size_t>> samples;
	list<Categoric_var*> vars;
	graph_3.Get_All_variables_in_model(&vars);

	graph_3.Gibbs_Sampling_on_Hidden_set(&samples, 500, 500);
	Print_set_as_training_set(prefix + "Train_set.txt", vars, samples);


	Random_Field graph_to_learn("graph_3_to_learn.xml", prefix); //in this model all weights are initially equal to 1
	Training_set Set(prefix + "Train_set.txt");
	auto Learner = I_Trainer::Get_fixed_step(1.f); // Random_Field::I_Training::Get_BFGS();  //
	list<float> likelihood_story;
	Learner->Train(&graph_to_learn, &Set, 50, &likelihood_story);
	delete Learner;

	cout << "\n\n\n evolution of the likelihood during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	list<float> w;
	cout << "\n\n\n real weights of the model\n";
	Graph_Learnable::Weights_Manager::Get_tunable_w(&w, &graph_3);
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	Graph_Learnable::Weights_Manager::Get_tunable_w(&w, &graph_to_learn);
	print_distribution(w); cout << endl;
	cout << endl;


	list<float> marginals;
	graph_3.Set_Observation_Set_var({ graph_3.Find_Variable("v5") });
	graph_3.Set_Observation_Set_val({ 0 });
	graph_3.Get_marginal_distribution(&marginals, graph_3.Find_Variable("v1"));
	cout << "P(A|C=0) real model    ";
	print_distribution(marginals); cout << endl;

	graph_to_learn.Set_Observation_Set_var({ graph_to_learn.Find_Variable("v5") });
	graph_to_learn.Set_Observation_Set_val({ 0 });
	graph_to_learn.Get_marginal_distribution(&marginals, graph_to_learn.Find_Variable("v1"));
	cout << "P(A|C=0) learnt model  ";
	print_distribution(marginals); cout << endl;

}
