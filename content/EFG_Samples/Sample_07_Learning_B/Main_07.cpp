/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

 // This example is also explained in the documentation (the .pdf file), Section 'Learning of graphs, part B'

#include <iostream>
#include <string>
#include <memory>
using namespace std;

#include "../../EFG/Header/Graphical_model.h"
#include "../../EFG/Header/Trainer.h"
#include "../Utilities.h"
using namespace EFG;

unique_ptr<Training_set> Create_training_set(Conditional_Random_Field& Model);

vector<float> ones(const size_t& Size);

int main() {

//build the conditional random field reported in Sample 06
	Conditional_Random_Field cond_graph(string("./Sample_07_graphs/cond_graph.xml"));

// build a similar structure with all 1 as weights and then learn them using the previous training set
	Conditional_Random_Field cond_graph_to_learn(cond_graph); 
	// set all tuanble weights equal to 1
	cond_graph_to_learn.Set_tunable(ones(cond_graph_to_learn.Get_model_size()));

	auto Set = Create_training_set(cond_graph);
	auto Learner = I_Trainer::Get_fixed_step(1.f, 0.3f); //the stochastic gradient descend version is used: at every iteration 30% of the samples in the training set are used to evaluate the gradient
	list<float> likelihood_story;
	Learner->Train(cond_graph_to_learn, *Set.get(), 10, &likelihood_story);

	cout << "\n\n\n evolution of the likelihood during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	vector<float> w;
	cout << "\n\n\n real weights of the model\n";
	cond_graph.Get_tunable(&w);
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	cond_graph_to_learn.Get_tunable(&w);
	print_distribution(w); cout << endl;
	cout << endl;


// compare the marginals computation of the real and the learnt models
// adopting as observations the values {0,1,0,1,...}
	list<size_t> ob_val;
	list<Categoric_var*> ob_var;
	cond_graph_to_learn.Get_Actual_Observation_Set_Var(&ob_var);
	bool temp = false;
	for (size_t k = 0; k < ob_var.size(); k++) {
		ob_val.push_back((size_t)temp);
		temp = !temp;
	}

	vector<float> marginals;
	cond_graph.Set_Evidences(ob_val);
	cond_graph.Get_marginal_distribution(&marginals, cond_graph.Find_Variable("Y4"));
	cout << "P(Y|X) real model    ";
	print_distribution(marginals); cout << endl;

	cond_graph_to_learn.Set_Evidences(ob_val);
	cond_graph_to_learn.Get_marginal_distribution(&marginals, cond_graph_to_learn.Find_Variable("Y4"));
	cout << "P(Y|X) learnt model  ";
	print_distribution(marginals); cout << endl;

	return 0;
}

template<class T>
void Append_list(list<T>* result, const list<T>& to_append) {

	for (auto it = to_append.begin(); it != to_append.end(); it++)
		result->push_back(*it);

}

void get_names(list<string>* names, const list<Categoric_var*>& vars) {

	names->clear();
	for (auto it = vars.begin(); it != vars.end(); ++it)
		names->push_back((*it)->Get_name());

}

unique_ptr<Training_set> Create_training_set(Conditional_Random_Field& Model) {

	// get the variables in the hidden and observed sets
	list<Categoric_var*> hidden_set;
	Model.Get_Actual_Hidden_Set(&hidden_set);
	list<Categoric_var*> observed_set;
	Model.Get_Actual_Observation_Set_Var(&observed_set);

	//compute all the possible combinations of observations to build a training set, considering 
	// a Gibbs sampling method for each observation
	I_Potential::combinations obs_comb(observed_set);
	I_Potential::combinations::iterator obs_it(obs_comb);
	list<list<size_t>> samples;
	list<list<size_t>> samples_temp;
	list<size_t> obs_as_list;
	size_t k, K = observed_set.size();
	const size_t* c;
	while (obs_it.is_not_at_end()) {
		obs_as_list.clear();
		c = *obs_it;
		for (k = 0; k < K; k++)
			obs_as_list.push_back(c[k]);
		Model.Set_Evidences(obs_as_list);
		Model.Gibbs_Sampling_on_Hidden_set(&samples_temp, 15, 100);
		for (auto itt = samples_temp.begin(); itt != samples_temp.end(); itt++) {
			Append_list(&(*itt), obs_as_list);
			samples.push_back(*itt);
		}
		++obs_it;
	}

	list<string> names , obs_names;
	get_names(&names, hidden_set);
	get_names(&obs_names, observed_set);
	Append_list(&names, obs_names);

	return unique_ptr<Training_set>(new Training_set(samples, names));

}

vector<float> ones(const size_t& Size) {

	vector<float> temp;
	temp.reserve(Size);
	for (size_t k = 0; k < Size; k++)
		temp.push_back(1.f);
	return temp;

}