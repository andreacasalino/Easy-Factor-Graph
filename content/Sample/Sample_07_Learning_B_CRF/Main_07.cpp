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

template<class T>
void Append_list(list<T>* result, const list<T>& to_append) {

	for (auto it = to_append.begin(); it != to_append.end(); it++)
		result->push_back(*it);

}

int main() {
	string prefix =  compute_prefix() + "Sample_07_Learning_B_CRF" + "/";

//build conditional random field
	Conditional_Random_Field cond_graph("cond_graph.xml", prefix);

	list<Categoric_var*> hidden_set;
	cond_graph.Get_Actual_Hidden_Set(&hidden_set);
	list<Categoric_var*> observed_set;
	cond_graph.Get_Actual_Observation_Set(&observed_set);

//extract some samples from the graph with a Gibbs sampling method, for building a train set

	list<list<size_t>> samples;
	list<list<size_t>> samples_temp;

	//compute all the possible combinations of observations
	list<list<size_t>> domain_of_observed_set;
	I_Potential::Get_entire_domain(&domain_of_observed_set, observed_set);
	for (auto it_o = domain_of_observed_set.begin(); it_o != domain_of_observed_set.end(); it_o++) {
		cond_graph.Set_Observation_Set_val(*it_o);
		cond_graph.Gibbs_Sampling_on_Hidden_set(&samples_temp, 50, 100);
		for (auto itt = samples_temp.begin(); itt != samples_temp.end(); itt++)
			Append_list(&(*itt), *it_o);

		Append_list(&samples, samples_temp);
	}
	list<Categoric_var*> Entire_set = hidden_set;
	Append_list(&Entire_set, observed_set);
	Print_set_as_training_set(prefix + "Train_set.txt", Entire_set, samples);


	Conditional_Random_Field cond_graph_to_learn("cond_graph_to_learn.xml", prefix); //in this model all weights are initially equal to 1
	Training_set Set(prefix + "Train_set.txt");
	auto Learner = I_Trainer::Get_fixed_step(0.1f); // Random_Field::I_Training::Get_BFGS();  //
	list<float> likelihood_story;
	Learner->Train(&cond_graph_to_learn, &Set, 5, &likelihood_story);
	delete Learner;

	cout << "\n\n\n evolution of the likelihood during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	list<float> w;
	cout << "\n\n\n real weights of the model\n";
	Graph_Learnable::Weights_Manager::Get_w(&w, &cond_graph);
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	Graph_Learnable::Weights_Manager::Get_w(&w, &cond_graph_to_learn);
	print_distribution(w); cout << endl;
	cout << endl;

	system("pause");
	return 0;
}
