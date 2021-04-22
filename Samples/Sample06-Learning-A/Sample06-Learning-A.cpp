/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// Refer also to Section 'Sample 06: Learning, part A' of the documentation


#include <model/RandomField.h>
#include <train/trainers/GradientDescend.h>
#include <io/xml/Importer.h>
#include <print/ProbabilityDistributionPrint.h>
#include <Presenter.h>
#include <Frequencies.h>
#include <math.h>
#include <iostream>
using namespace std;
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::train;

int main() {
	EFG::sample::samplePart([]() {
		//build the structure explained in 'Sample 06: Learning, part A / part 01'
		VariablePtr A = makeVariable(2, "A");
		VariablePtr B = makeVariable(2, "B");
		VariablePtr C = makeVariable(2, "C");

		float alfa = 1.f, beta = 3.f, gamma = 0.1f;

		model::RandomField graph; //the potentials will be moved
		graph.Insert(factor::cnst::FactorExponential(factor::cnst::Factor(categoric::Group(A, B), true) , alfa));
		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(A, C), true) , beta));
		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(B, C), true) , gamma));

		//extract some samples form the joint distributions of the variable in the graph, using a Gibbs sampling method
		auto samples = graph.getHiddenSetSamples(500, 500);

		float Z = 2.f*(expf(alfa) + expf(beta) + expf(gamma) + expf(alfa)*expf(beta)*expf(gamma));
		auto hidden_set = graph.getHiddenVariables();
		cout << "Gibbs sampler results\n";
		cout << "freq <0,0,0> " << sample::getEmpiricalFrequencies(Combination({0,0,0}), Group(A, B, C), samples, Group(A, B, C).getVariables()) << expf(alfa)*expf(beta)*expf(gamma) / Z << endl;
		cout << "freq <1,0,0> " << sample::getEmpiricalFrequencies(Combination({0,0,0}), Group(A, B, C), samples, Group(A, B, C).getVariables()) << expf(gamma) / Z << endl;
		cout << "freq <0,1,0> " << sample::getEmpiricalFrequencies(Combination({0,0,0}), Group(A, B, C), samples, Group(A, B, C).getVariables()) << expf(beta) / Z << endl;
		cout << "freq <1,1,0> " << sample::getEmpiricalFrequencies(Combination({0,0,0}), Group(A, B, C), samples, Group(A, B, C).getVariables()) << expf(alfa) / Z << endl;

		//build a second graph, with the same potentials, but all weights equal to 1. Then use the train set made by the previous samples to train 
		//this model, for obtaining a combination of weights similar to the original one
		model::RandomField graph2Learn;
		graph2Learn.absorbStructure(static_cast<const nodes::StructureAware&>(graph), true);
		graph2Learn.absorbStructureTunable(static_cast<const nodes::StructureTunableAware&>(graph), true);
		graph2Learn.setOnes();

		TrainSetPtr trainSet = std::make_shared<TrainSet>(samples);
		GradientDescend trainer;
		trainer.setAdvancement(0.2f);

		cout << "\n\n\n real weights of the model\n";
		cout << graph.getWeights() << endl;

		cout << "\n\n\n learnt weights\n";
		cout << graph2Learn.getWeights() << endl;

		//compare the marginals distributions of the real model and the learnt one
        cout << "P(A|C=0)\n";

        cout << "real model " << endl;
		graph.resetEvidences(map<string, size_t>{ {"C", 0}});
        cout << graph.getMarginalDistribution("A") << endl << endl;

        cout << "learnt model " << endl;
		graph2Learn.resetEvidences(map<string, size_t>{ {"C", 0}});
        cout << graph2Learn.getMarginalDistribution("A") << endl << endl;
	}, "Simple tunable model");

	EFG::sample::samplePart([]() {
		
	}, "Medium size tunable model");

	EFG::sample::samplePart([]() {
		
	}, "Complex tunable model");

	EFG::sample::samplePart([]() {
		
	}, "Model with shared weights");

	return EXIT_SUCCESS;
}




// void set_ones_tunable(model::RandomField& model);

// vector<string> get_names(const vector<CategoricVariable*>& vars);

// string get_train_folder();

// void part_02() {

// 	//build the structure explained in 'Sample 06: Learning, part A / part 02'
// 	CategoricVariable A(2, "A"); CategoricVariable B(2, "B"); CategoricVariable C(2, "C"); CategoricVariable D(2, "D"); CategoricVariable E(2, "E");

// 	float alfa = 0.4f, beta = 1.f, gamma = 0.3f, delta = 1.5f;

// 	model::RandomField graph_2; //the potentials will be moved
// 	//the unique_pointer are used in order to realize that the passed potentials are destroyed after the Inesertion ends 
// 	graph_2.InsertMove(pot::ExpFactor::makeFactor(alfa, vector<CategoricVariable*>{ &A,&B }, true), false); // the weight of this potential will be kept constant
// 	graph_2.InsertMove(pot::ExpFactor::makeFactor(beta, vector<CategoricVariable*>{ &A, & C }, true));
// 	graph_2.InsertMove(pot::Factor(vector<CategoricVariable*>{ &B,&C }, true));
// 	graph_2.InsertMove(pot::ExpFactor::makeFactor(gamma, vector<CategoricVariable*>{ &B,&E }, true), false); // the weight of this potential will be kept constant
// 	graph_2.InsertMove(pot::ExpFactor::makeFactor(delta, vector<CategoricVariable*>{ &B,&D }, true));
// 	graph_2.InsertMove(pot::Factor(vector<CategoricVariable*>{ &D,&E }, true));

// 	//export into an xml the built graph (just to show the syntax that would have been required to defined the same stucture into an xml)
// 	graph_2.Reprint(Get_prefix() + "Sample_06_graphs/graph_2_printed.xml");

// 	//extract some samples from the graph with a Gibbs sampling method, for building a train set
// 	list<vector<size_t>> samples = graph_2.GibbsSamplingHiddenSet(1000, 500);

// 	//build a second graph, with the same potentials, but all weights equal to 1, a part from the constant (known) ones. Then use the previous train set to train 
// 	//this model, for obtaining a combination of weights similar to the original ones

// 	model::RandomField graph_to_learn; //all the future insertion will be cloning
// 	graph_to_learn.Insert(graph_2.GetStructure(), false);
// 	set_ones_tunable(graph_to_learn);

// #ifdef USE_TEXTUAL_TRAIN_SET
// 	Print_set_as_training_set(get_train_folder(), graph_2.GetHiddenSet(), samples);
// 	train::TrainingSet Set(get_train_folder());
// #else
// 	train::TrainingSet Set(get_names(graph_2.GetHiddenSet()), samples);
// #endif

// 	//train graph_to_learn with a fixed step gradient descend algorithm
// 	auto Learner = train::Trainer::GetFixedStep(1.f);
// 	list<float> likelihood_story;
// 	(*Learner)(graph_to_learn, Set, 70, &likelihood_story);

// 	cout << "\n\n\n evolution of the likelihood of the model during training\n";
// 	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
// 		cout << *it << endl;

// 	vector<float> w;
// 	cout << "\n\n\n real weights of the model\n";
// 	w = graph_2.GetTunable();//only the second and fourth weights are returned, i.e. the tunable ones
// 	print_distribution(w); cout << endl;
// 	cout << "\n\n\n learnt weights\n";
// 	w = graph_to_learn.GetTunable();//only the second and fourth weights are returned, i.e. the tunable ones
// 	print_distribution(w); cout << endl;
// 	cout << endl;

// 	//compare the marginals distributions of the real model and the learnt one
// 	vector<float> marginals;
// 	graph_2.SetEvidences(vector<pair<string, size_t>>{ {"D", 0}});
// 	marginals = graph_2.GetMarginalDistribution("A");
// 	cout << "P(A|D=0) real model    ";
// 	print_distribution(marginals); cout << endl;

// 	graph_to_learn.SetEvidences(vector<pair<string, size_t>>{ {"D", 0}});
// 	marginals = graph_to_learn.GetMarginalDistribution("A");
// 	cout << "P(A|D=0) learnt model  ";
// 	print_distribution(marginals); cout << endl;
// }

// void part_03() {

// 	//build the structure explained in 'Sample 06: Learning, part A / part 03'
// 	model::RandomField graph_3(Get_prefix() + "Sample_06_graphs/graph_3.xml");

// 	//extract some samples from the graph with a Gibbs sampling method, for building a train set
// 	list<vector<size_t>> samples = graph_3.GibbsSamplingHiddenSet(500, 500);

// 	model::RandomField graph_to_learn;
// 	graph_to_learn.Insert(graph_3.GetStructure(), false);
// 	//set all weights equal to 1
// 	set_ones_tunable(graph_to_learn);
// #ifdef THREAD_POOL_ENABLED
// 	graph_to_learn.SetThreadPoolSize(3);
// #endif

// #ifdef USE_TEXTUAL_TRAIN_SET
// 	Print_set_as_training_set(get_train_folder(), graph_3.GetHiddenSet(), samples);
// 	train::TrainingSet Set(get_train_folder());
// #else
// 	train::TrainingSet Set(get_names(graph_3.GetHiddenSet()), samples);
// #endif

// 	//tune the weights according to the sampled train set
// 	auto Learner = train::Trainer::GetFixedStep(1.f); // model::RandomField::I_Training::Get_BFGS();  //
// 	list<float> likelihood_story;
// 	(*Learner)(graph_to_learn, Set, 50, &likelihood_story);

// 	cout << "\n\n\n evolution of the likelihood during training\n";
// 	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
// 		cout << *it << endl;

// 	vector<float> w;
// 	cout << "\n\n\n real weights of the model\n";
// 	w = graph_3.GetTunable();
// 	print_distribution(w); cout << endl;
// 	cout << "\n\n\n learnt weights\n";
// 	w = graph_to_learn.GetTunable();
// 	print_distribution(w); cout << endl;
// 	cout << endl;


// 	//compare the marginals distributions of the real model and the learnt one
// 	vector<float> marginals;
// 	graph_3.SetEvidences(vector<pair<string, size_t>>{ {"v5", 0}});
// 	marginals = graph_3.GetMarginalDistribution("v1");
// 	cout << "P(v1|v5=0) real model    ";
// 	print_distribution(marginals); cout << endl;

// 	graph_to_learn.SetEvidences(vector<pair<string, size_t>>{ {"v5", 0}});
// 	marginals = graph_to_learn.GetMarginalDistribution("v1");
// 	cout << "P(v1|v5=0) learnt model  ";
// 	print_distribution(marginals); cout << endl;

// }

// void part_04() {

// 	//build the structure explained in 'Sample 06: Learning, part A / part 04'
// 	float alfa = 2.f;
// 	float beta = 1.f;

// 	CategoricVariable Y1(2, "Y1"); CategoricVariable Y2(2, "Y2"); CategoricVariable Y3(2, "Y3");
// 	CategoricVariable X1(2, "X1"); CategoricVariable X2(2, "X2"); CategoricVariable X3(2, "X3");

// 	model::RandomField graph_5;
// 	graph_5.InsertMove(pot::Factor(vector<CategoricVariable*>{ &Y1, & X1 }, true), beta);
// 	graph_5.InsertMove(pot::Factor(vector<CategoricVariable*>{ &Y2, & X2 }, true), { "Y1", "X1" }); // the same weight of XY1 is assumed
// 	graph_5.InsertMove(pot::Factor(vector<CategoricVariable*>{ &Y3, & X3 }, true), { "X1", "Y1" }); // the same weight of XY1 is assumed
// 	graph_5.InsertMove(pot::Factor(vector<CategoricVariable*>{ &Y1, & Y2 }, true), alfa);
// 	graph_5.InsertMove(pot::Factor(vector<CategoricVariable*>{ &Y2, & Y3 }, true), { "Y1", "Y2" }); // the same weight of YY1 is assumed

// 	//extract some samples from the graph with a Gibbs sampling method, for building a train set
// 	list<vector<size_t>> samples = graph_5.GibbsSamplingHiddenSet(500, 500);


// 	//build a model having the same structure (sharing of potential) with different values for the weight
// 	model::RandomField graph_to_learn;
// 	graph_to_learn.Insert(graph_5.GetStructure(), false);
// 	//set all weights equal to 1
// 	set_ones_tunable(graph_to_learn);

// #ifdef USE_TEXTUAL_TRAIN_SET
// 	Print_set_as_training_set(get_train_folder(), graph_5.GetHiddenSet(), samples);
// 	train::TrainingSet Set(get_train_folder());
// #else
// 	train::TrainingSet Set(get_names(graph_5.GetHiddenSet()), samples);
// #endif

// 	//learn the model, considering the constraints about the weights sharing
// 	auto Learner = train::Trainer::GetFixedStep(1.f); // model::RandomField::I_Training::Get_BFGS();  //
// 	list<float> likelihood_story;
// 	(*Learner)(graph_to_learn, Set, 50, &likelihood_story);

// 	cout << "\n\n\n evolution of the likelihood during training\n";
// 	for (auto it = likelihood_story.begin(); it != likelihood_story.end(); it++)
// 		cout << *it << endl;

// 	vector<float> w;
// 	cout << "\n\n\n real weights of the model\n";
// 	w = graph_5.GetTunable();//only and alfa and beta are free parameters
// 	print_distribution(w); cout << endl;
// 	cout << "\n\n\n learnt weights\n";
// 	w = graph_to_learn.GetTunable();
// 	print_distribution(w); cout << endl;
// 	cout << endl;

// }

// void set_ones_tunable(model::RandomField& model) {

// 	size_t S = model.GetModelSize();
// 	vector<float> w_tunab;
// 	w_tunab.reserve(S);
// 	for (size_t k = 0; k < S; k++)
// 		w_tunab.push_back(1.f);
// 	model.SetTunable(w_tunab);

// }

// vector<string> get_names(const vector<CategoricVariable*>& vars) {
// 	vector<string> n;
// 	n.reserve(vars.size());
// 	for (auto it : vars)  n.push_back(it->GetName());
// 	return n;
// }
