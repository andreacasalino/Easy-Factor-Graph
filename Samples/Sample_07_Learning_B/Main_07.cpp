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

#include <model/ConditionalRandomField.h>
#include <train/Trainer.h>
#include "../Utilities.h"
using namespace EFG;

unique_ptr<train::TrainingSet> Create_training_set(model::ConditionalRandomField& Model);

vector<float> ones(const std::size_t& Size);

int main() {

//build the conditional random field reported in Sample 06
	model::ConditionalRandomField cond_graph(Get_prefix() + "Sample_07_graphs/cond_graph.xml");

// build a similar structure with all 1 as weights and then learn them using the previous training set
	model::ConditionalRandomField cond_graph_to_learn(static_cast<const node::Node::NodeFactory&>(cond_graph));
	// set all tuanble weights equal to 1
	cond_graph_to_learn.SetTunable(ones(cond_graph_to_learn.GetModelSize()));

	auto Set = Create_training_set(cond_graph);
	auto Learner = train::Trainer::GetFixedStep(1.f, 0.3f); //the stochastic gradient descend version is used: at every iteration 30% of the samples in the training set are used to evaluate the gradient
	list<float> likelihood_story;
	(*Learner)(cond_graph_to_learn, *Set.get(), 10, &likelihood_story);

	cout << "\n\n\n evolution of the likelihood during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	vector<float> w;
	cout << "\n\n\n real weights of the model\n";
	w = cond_graph.GetTunable();
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	w = cond_graph_to_learn.GetTunable();
	print_distribution(w); cout << endl;
	cout << endl;


// compare the marginals computation of the real and the learnt models
// adopting as observations the values {0,1,0,1,...}
	size_t O = cond_graph_to_learn.GetObservationSet().size();
	vector<size_t> obs;
	obs.reserve(O);
	bool temp = false;
	for (size_t k = 0; k < O; k++) {
		obs.push_back((size_t)temp);
		temp = !temp;
	}

	vector<float> marginals;
	cond_graph.SetEvidences(obs);
	marginals = cond_graph.GetMarginalDistribution("Y4");
	cout << "P(Y|X) real model    ";
	print_distribution(marginals); cout << endl;

	cond_graph_to_learn.SetEvidences(obs);
	marginals = cond_graph_to_learn.GetMarginalDistribution("Y4");
	cout << "P(Y|X) learnt model  ";
	print_distribution(marginals); cout << endl;

	return 0;
}

unique_ptr<train::TrainingSet> Create_training_set(model::ConditionalRandomField& Model) {

	// get the variables in the hidden and observed sets
	vector<CategoricVariable*> hidden_set = Model.GetHiddenSet();
	vector<CategoricVariable*> observed_set;
	{
		auto temp = Model.GetObservationSet();
		observed_set.reserve(temp.size());
		for (size_t k = 0; k < temp.size(); ++k) observed_set.push_back(temp[k].first);
	}
	vector<string> names;
	names.reserve(hidden_set.size() + observed_set.size());
	for (auto it : hidden_set) names.push_back(it->GetName());
	for (auto it : observed_set) names.push_back(it->GetName());

	//compute all the possible combinations of observations to build a training set, considering 
	// a Gibbs sampling method for each observation
	list<vector<size_t>> samples;
	JointDomainIterator::forEach(observed_set, [&Model, &samples, &observed_set](const vector<size_t>& comb) {
		Model.SetEvidences(comb);
		auto hidden_sampled = Model.GibbsSamplingHiddenSet(15, 100);
		while (!hidden_sampled.empty()) {
			samples.emplace_back(std::move(hidden_sampled.front()));
			hidden_sampled.pop_front();
			samples.back().reserve(samples.back().size() + observed_set.size());
			for (auto it : comb) samples.back().push_back(it);
		}
	});

	return make_unique<train::TrainingSet>(names , samples);

}

vector<float> ones(const std::size_t& Size) {

	vector<float> temp;
	temp.reserve(Size);
	for (size_t k = 0; k < Size; k++)
		temp.push_back(1.f);
	return temp;

}