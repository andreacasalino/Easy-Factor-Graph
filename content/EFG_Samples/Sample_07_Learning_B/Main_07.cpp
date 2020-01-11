/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

 // This example is also explained in the documentation (the .pdf file), Section 'Learning of graphs, part B'

#include <iostream>
#include <string>
using namespace std;

#include "../../EFG/Header/Graphical_model.h"
#include "../../EFG/Header/Trainer.h"
#include "../Utilities.h"
using namespace EFG;

template<class T>
void Append_list(list<T>* result, const list<T>& to_append) {

	for (auto it = to_append.begin(); it != to_append.end(); it++)
		result->push_back(*it);

}

int main() {

//build the conditional random field reported in Sample 06
	Conditional_Random_Field cond_graph(string("./Sample_07_graphs/cond_graph.xml"));

// get the variables in the hidden and observed sets
	list<Categoric_var*> hidden_set;
	cond_graph.Get_Actual_Hidden_Set(&hidden_set);
	list<Categoric_var*> observed_set;
	cond_graph.Get_Actual_Observation_Set_Var(&observed_set);

//extract some samples from the graph with a Gibbs sampling method, for building a train set
	list<list<size_t>> samples;
	list<list<size_t>> samples_temp;

	//compute all the possible combinations of observations
	list<list<size_t>> domain_of_observed_set;
	I_Potential::Get_entire_domain(&domain_of_observed_set, observed_set);
	for (auto it_o = domain_of_observed_set.begin(); it_o != domain_of_observed_set.end(); it_o++) {
		cond_graph.Set_Evidences(*it_o);
		cond_graph.Gibbs_Sampling_on_Hidden_set(&samples_temp, 15, 100);
		for (auto itt = samples_temp.begin(); itt != samples_temp.end(); itt++) 
			Append_list(&(*itt), *it_o);

		Append_list(&samples, samples_temp);
	}
	list<Categoric_var*> Entire_set = hidden_set;
	Append_list(&Entire_set, observed_set);
	Print_set_as_training_set("Train_set.txt", Entire_set, samples);


// build a similar structure with all 1 as weights and then learn them using the previous training set
	Conditional_Random_Field cond_graph_to_learn(string("./Sample_07_graphs/cond_graph_to_learn.xml")); //in this model all weights are initially equal to 1
	Training_set Set( "Train_set.txt");
	auto Learner = I_Trainer::Get_fixed_step(1.f, 0.3f);
	list<float> likelihood_story;
	Learner->Train(&cond_graph_to_learn, &Set, 10, &likelihood_story);
	delete Learner;

	cout << "\n\n\n evolution of the likelihood during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	list<float> w;
	cout << "\n\n\n real weights of the model\n";
	Graph_Learnable::Weights_Manager::Get_tunable_w(&w, &cond_graph);
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	Graph_Learnable::Weights_Manager::Get_tunable_w(&w, &cond_graph_to_learn);
	print_distribution(w); cout << endl;
	cout << endl;


// compare the marginals computation of the real and the learnt models
	list<float> marginals;
	cond_graph.Set_Evidences(domain_of_observed_set.front());
	cond_graph.Get_marginal_distribution(&marginals, cond_graph.Find_Variable("Y4"));
	cout << "P(Y|X) real model    ";
	print_distribution(marginals); cout << endl;

	cond_graph_to_learn.Set_Evidences(domain_of_observed_set.front());
	cond_graph_to_learn.Get_marginal_distribution(&marginals, cond_graph_to_learn.Find_Variable("Y4"));
	cout << "P(Y|X) learnt model  ";
	print_distribution(marginals); cout << endl;

	return 0;
}
