/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 06: Learning, part A' of the documentation



#include <iostream>
#include <string>
#include <memory>
using namespace std;

#include <Graphical_model.h>
#include <Trainer.h>
#include "../Utilities.h"
using namespace EFG;

//#define USE_TEXTUAL_TRAIN_SET


void part_01();
void part_02();
void part_03();
void part_04();

void set_ones_tunable(Random_Field& model);

vector<string> get_names(const vector<Categoric_var*>& vars);

string get_train_folder();

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

	///////////////////////////////////////////
	//            part 03 graph_4            //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 03 \n\n\n";
	cout << "-----------------------\n";
	part_03();

	///////////////////////////////////////////
	//            part 04 graph_5  , structure with shared weights          //
	///////////////////////////////////////////
	cout << "-----------------------\n";
	cout << "part 04 \n\n\n";
	cout << "-----------------------\n";
	part_04();

	return 0;
}

string get_train_folder() {
	return Get_prefix() + "Sample_06_graphs/Train_set.txt";
}

void part_01() {

	//build the structure explained in 'Sample 06: Learning, part A / part 01'
	Categoric_var A(2, "A"); Categoric_var B(2, "B"); Categoric_var C(2, "C");

	float alfa = 1.f, beta = 3.f, gamma = 0.1f;

	Potential_Exp_Shape Pot_AB(Potential_Shape(vector<Categoric_var*>{ &A,&B }, true), alfa);
	Potential_Exp_Shape Pot_AC(Potential_Shape(vector<Categoric_var*>{ &A,&C }, true), beta);
	Potential_Exp_Shape Pot_BC(Potential_Shape(vector<Categoric_var*>{ &B,&C }, true), gamma);

	Random_Field graph_2(false); //the potentials are not cloned, but simply absorbed
	graph_2.Insert(Pot_AB, false);
	graph_2.Insert(Pot_AC);
	graph_2.Insert(Pot_BC);

	//extract some samples form the joint distributions of the variable in the graph, using a Gibbs sampling method
	list<vector<size_t>> samples = graph_2.Gibbs_Sampling_on_Hidden_set(500, 500);

	//check empirical frequency for some combinations
	float Z = 2.f*(expf(alfa) + expf(beta) + expf(gamma) + expf(alfa)*expf(beta)*expf(gamma));

	vector<Categoric_var*> hidden_set = graph_2.Get_Actual_Hidden_Set();
	cout << "Gibbs sampler results\n";
	cout << "freq <0,0,0> " << Get_empirical_frequencies(samples, { 0,0,0 }, {graph_2.Find_Variable("A"),graph_2.Find_Variable("B"), graph_2.Find_Variable("C") }, hidden_set) << " theoretical: " << expf(alfa)*expf(beta)*expf(gamma) / Z << endl;
	cout << "freq <1,0,0> " << Get_empirical_frequencies(samples, { 1,0,0 }, { graph_2.Find_Variable("A"),graph_2.Find_Variable("B"), graph_2.Find_Variable("C") }, hidden_set) << " theoretical: " << expf(gamma) / Z << endl;
	cout << "freq <0,1,0> " << Get_empirical_frequencies(samples, { 0,1,0 }, { graph_2.Find_Variable("A"),graph_2.Find_Variable("B"), graph_2.Find_Variable("C") }, hidden_set) << " theoretical: " << expf(beta) / Z << endl;
	cout << "freq <1,1,0> " << Get_empirical_frequencies(samples, { 1,1,0 }, { graph_2.Find_Variable("A"),graph_2.Find_Variable("B"), graph_2.Find_Variable("C") }, hidden_set) << " theoretical: " << expf(alfa) / Z << endl;


	//build a second graph, with the same potentials, but all weights equal to 1. Then use the train set made by the previous samples to train 
	//this model, for obtaining a combination of weights similar to the original one
	Random_Field graph_to_learn; //the potentials are cloned here
	graph_to_learn.Insert(graph_2.Get_structure());
	graph_to_learn.Set_tunable({1.f, 1.f});

	//create the training set
#ifdef USE_TEXTUAL_TRAIN_SET
	Print_set_as_training_set(get_train_folder(), { &A,&B,&C }, samples);
	Training_set Set(get_train_folder());
#else
	Training_set Set(get_names(graph_2.Get_Actual_Hidden_Set()), samples);
#endif
	//train graph_to_learn with a fixed step gradient descend algorithm
	auto Learner = I_Trainer::Get_fixed_step(0.2f);
	list<float> likelihood_story;
	Learner->Train(graph_to_learn, Set, 100, &likelihood_story); 

	cout << "\n\n\n evolution of the likelihood of the model during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	vector<float> w;
	cout << "\n\n\n real weights of the model\n";
	w = graph_2.Get_tunable();
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	w = graph_to_learn.Get_tunable();
	print_distribution(w); cout << endl;
	cout << endl;

	//compare the marginals distributions of the real model and the learnt one
	vector<float> marginals;
	graph_2.Set_Evidences(vector<pair<string, size_t>>{ {"C", 0}});
	marginals = graph_2.Get_marginal_distribution("A");
	cout << "P(A|C=0) real model    ";
	print_distribution(marginals); cout << endl;

	graph_to_learn.Set_Evidences(vector<pair<string, size_t>>{ {"C", 0}});
	marginals = graph_to_learn.Get_marginal_distribution("A");
	cout << "P(A|C=0) learnt model  ";
	print_distribution(marginals); cout << endl;
}

void part_02() {

	//build the structure explained in 'Sample 06: Learning, part A / part 02'
	Categoric_var A(2, "A"); Categoric_var B(2, "B"); Categoric_var C(2, "C"); Categoric_var D(2, "D"); Categoric_var E(2, "E");

	float alfa = 0.4f, beta = 1.f, gamma = 0.3f, delta = 1.5f;

	Random_Field graph_2; //the potentials inserted will be cloned
	//the unique_pointer are used in order to realize that the passed potentials are destroyed after the Inesertion ends 
	graph_2.Insert(*unique_ptr<Potential_Exp_Shape>(new Potential_Exp_Shape(Potential_Shape(vector<Categoric_var*>{ &A,&B }, true), alfa)).get(), false); // the weight of this potential will be kept constant
	graph_2.Insert(*unique_ptr<Potential_Exp_Shape>(new Potential_Exp_Shape(Potential_Shape(vector<Categoric_var*>{ &A,&C }, true), beta)).get());
	graph_2.Insert(*unique_ptr<Potential_Shape>(new Potential_Shape(vector<Categoric_var*>{ &B,&C }, true)).get());
	graph_2.Insert(*unique_ptr<Potential_Exp_Shape>(new Potential_Exp_Shape(Potential_Shape(vector<Categoric_var*>{ &B,&E }, true), gamma)).get(), false); // the weight of this potential will be kept constant
	graph_2.Insert(*unique_ptr<Potential_Exp_Shape>(new Potential_Exp_Shape(Potential_Shape(vector<Categoric_var*>{ &B,&D }, true), delta)).get());
	graph_2.Insert(*unique_ptr<Potential_Shape>(new Potential_Shape(vector<Categoric_var*>{ &D,&E }, true)).get());

	//export into an xml the built graph (just to show the syntax that would have been required to defined the same stucture into an xml)
	graph_2.Reprint(Get_prefix() + "Sample_06_graphs/graph_2_printed.xml");

	//extract some samples from the graph with a Gibbs sampling method, for building a train set
	list<vector<size_t>> samples = graph_2.Gibbs_Sampling_on_Hidden_set(1000, 500);

	//build a second graph, with the same potentials, but all weights equal to 1, a part from the constant (known) ones. Then use the previous train set to train 
	//this model, for obtaining a combination of weights similar to the original ones

	Random_Field graph_to_learn; //all the future insertion will be cloning
	graph_to_learn.Insert(graph_2.Get_structure());
	set_ones_tunable(graph_to_learn);

#ifdef USE_TEXTUAL_TRAIN_SET
	Print_set_as_training_set(get_train_folder(), graph_2.Get_Actual_Hidden_Set(), samples);
	Training_set Set(get_train_folder());
#else
	Training_set Set(get_names(graph_2.Get_Actual_Hidden_Set()), samples);
#endif

	//train graph_to_learn with a fixed step gradient descend algorithm
	auto Learner = I_Trainer::Get_fixed_step(1.f);
	list<float> likelihood_story;
	Learner->Train(graph_to_learn, Set, 70, &likelihood_story);

	cout << "\n\n\n evolution of the likelihood of the model during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	vector<float> w;
	cout << "\n\n\n real weights of the model\n";
	w = graph_2.Get_tunable();//only the second and fourth weights are returned, i.e. the tunable ones
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	w = graph_to_learn.Get_tunable();//only the second and fourth weights are returned, i.e. the tunable ones
	print_distribution(w); cout << endl;
	cout << endl;

	//compare the marginals distributions of the real model and the learnt one
	vector<float> marginals;
	graph_2.Set_Evidences(vector<pair<string, size_t>>{ {"D", 0}});
	marginals = graph_2.Get_marginal_distribution("A");
	cout << "P(A|D=0) real model    ";
	print_distribution(marginals); cout << endl;

	graph_to_learn.Set_Evidences(vector<pair<string, size_t>>{ {"D", 0}});
	marginals = graph_to_learn.Get_marginal_distribution("A");
	cout << "P(A|D=0) learnt model  ";
	print_distribution(marginals); cout << endl;
}

void part_03() {

	//build the structure explained in 'Sample 06: Learning, part A / part 03'
	Random_Field graph_3(Get_prefix() + "Sample_06_graphs/graph_3.xml");

	//extract some samples from the graph with a Gibbs sampling method, for building a train set
	list<vector<size_t>> samples = graph_3.Gibbs_Sampling_on_Hidden_set(500, 500);

	Random_Field graph_to_learn;
	graph_to_learn.Insert(graph_3.Get_structure());
	//set all weights equal to 1
	set_ones_tunable(graph_to_learn);

#ifdef USE_TEXTUAL_TRAIN_SET
	Print_set_as_training_set(get_train_folder(), graph_3.Get_Actual_Hidden_Set(), samples);
	Training_set Set(get_train_folder());
#else
	Training_set Set(get_names(graph_3.Get_Actual_Hidden_Set()), samples);
#endif

	//tune the weights according to the sampled train set
	auto Learner = I_Trainer::Get_fixed_step(1.f); // Random_Field::I_Training::Get_BFGS();  //
	list<float> likelihood_story;
	Learner->Train(graph_to_learn, Set, 50, &likelihood_story);

	cout << "\n\n\n evolution of the likelihood during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	vector<float> w;
	cout << "\n\n\n real weights of the model\n";
	w = graph_3.Get_tunable();
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	w = graph_to_learn.Get_tunable();
	print_distribution(w); cout << endl;
	cout << endl;


	//compare the marginals distributions of the real model and the learnt one
	vector<float> marginals;
	graph_3.Set_Evidences(vector<pair<string, size_t>>{ {"v5", 0}});
	marginals = graph_3.Get_marginal_distribution("v1");
	cout << "P(v1|v5=0) real model    ";
	print_distribution(marginals); cout << endl;

	graph_to_learn.Set_Evidences(vector<pair<string, size_t>>{ {"v5", 0}});
	marginals = graph_to_learn.Get_marginal_distribution("v1");
	cout << "P(v1|v5=0) learnt model  ";
	print_distribution(marginals); cout << endl;

}

void part_04() {

	//build the structure explained in 'Sample 06: Learning, part A / part 04'
	float alfa = 2.f;
	float beta = 1.f;

	Categoric_var Y1(2, "Y1"); Categoric_var Y2(2, "Y2"); Categoric_var Y3(2, "Y3");
	Categoric_var X1(2, "X1"); Categoric_var X2(2, "X2"); Categoric_var X3(2, "X3");

	Potential_Exp_Shape XY1(Potential_Shape(vector<Categoric_var*>{ &Y1, &X1 }, true), beta);
	Potential_Exp_Shape XY2(Potential_Shape(vector<Categoric_var*>{ &Y2, &X2 }, true));
	Potential_Exp_Shape XY3(Potential_Shape(vector<Categoric_var*>{ &Y3, &X3 }, true));
	Potential_Exp_Shape YY1(Potential_Shape(vector<Categoric_var*>{ &Y1, &Y2 }, true), alfa);
	Potential_Exp_Shape YY2(Potential_Shape(vector<Categoric_var*>{ &Y2, &Y3 }, true));
	Random_Field graph_5(false); //potentials are not cloned when Insert is used
	graph_5.Insert(XY1);
	graph_5.Insert(XY2, { "Y1", "X1" }); // the same weight of XY1 is assumed
	graph_5.Insert(XY3, { "X1", "Y1" }); // the same weight of XY1 is assumed
	graph_5.Insert(YY1);
	graph_5.Insert(YY2, { "Y1", "Y2" }); // the same weight of YY1 is assumed

	//extract some samples from the graph with a Gibbs sampling method, for building a train set
	list<vector<size_t>> samples = graph_5.Gibbs_Sampling_on_Hidden_set(500, 500);


	//build a model having the same structure (sharing of potential) with different values for the weight
	Random_Field graph_to_learn;
	graph_to_learn.Insert(graph_5.Get_structure());
	//set all weights equal to 1
	set_ones_tunable(graph_to_learn);

#ifdef USE_TEXTUAL_TRAIN_SET
	Print_set_as_training_set(get_train_folder(), graph_5.Get_Actual_Hidden_Set(), samples);
	Training_set Set(get_train_folder());
#else
	Training_set Set(get_names(graph_5.Get_Actual_Hidden_Set()), samples);
#endif

	//learn the model, considering the constraints about the weights sharing
	auto Learner = I_Trainer::Get_fixed_step(1.f); // Random_Field::I_Training::Get_BFGS();  //
	list<float> likelihood_story;
	Learner->Train(graph_to_learn, Set, 50, &likelihood_story);

	cout << "\n\n\n evolution of the likelihood during training\n";
	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
		cout << *it << endl;

	vector<float> w;
	cout << "\n\n\n real weights of the model\n";
	w = graph_5.Get_tunable();//only and alfa and beta are free parameters
	print_distribution(w); cout << endl;
	cout << "\n\n\n learnt weights\n";
	w = graph_to_learn.Get_tunable();
	print_distribution(w); cout << endl;
	cout << endl;

}

void set_ones_tunable(Random_Field& model) {

	size_t S = model.Get_model_size();
	vector<float> w_tunab;
	w_tunab.reserve(S);
	for (size_t k = 0; k < S; k++)
		w_tunab.push_back(1.f);
	model.Set_tunable(w_tunab);

}

vector<string> get_names(const vector<Categoric_var*>& vars) {
	vector<string> n;
	n.reserve(vars.size());
	for (auto it : vars)  n.push_back(it->Get_name());
	return n;
}
