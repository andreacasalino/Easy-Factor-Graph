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
using namespace EFG::io;
using namespace EFG::train;

void trainModel(model::RandomField& graph, TrainSetPtr trainSet, const std::pair<string, size_t>& checkObservation, const string& checkVariable);

int main() {
	EFG::sample::samplePart([]() {
		//build the structure explained in 'Sample 06: Learning, part A / part 01'
		VariablePtr A = makeVariable(2, "A");
		VariablePtr B = makeVariable(2, "B");
		VariablePtr C = makeVariable(2, "C");

		float alfa = 1.f, beta = 3.f, gamma = 0.1f;

		model::RandomField graph;
		graph.Insert(factor::cnst::FactorExponential(factor::cnst::Factor(categoric::Group(A, B), true) , alfa)); // the weight of this potential will be kept constant
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

		trainModel(graph, std::make_shared<TrainSet>(samples), std::make_pair("C" , 0), "A");
	}, "Simple tunable model");

	EFG::sample::samplePart([]() {
		//build the structure explained in 'Sample 06: Learning, part A / part 02'
		VariablePtr A = makeVariable(2, "A");
		VariablePtr B = makeVariable(2, "B");
		VariablePtr C = makeVariable(2, "C");
		VariablePtr D = makeVariable(2, "D");
		VariablePtr E = makeVariable(2, "E");

		float alfa = 0.4f, beta = 1.f, gamma = 0.3f, delta = 1.5f;

		model::RandomField graph;
		graph.Insert(factor::cnst::FactorExponential(factor::cnst::Factor(categoric::Group(A, B), true) , alfa)); // the weight of this potential will be kept constant
		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(A, C), true) , beta));
		graph.Insert(factor::cnst::Factor(categoric::Group(B, C), true));
		graph.Insert(factor::cnst::FactorExponential(factor::cnst::Factor(categoric::Group(B, E), true) , gamma)); // the weight of this potential will be kept constant
		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(B, D), true) , delta));
		graph.Insert(factor::cnst::Factor(categoric::Group(D, E), true));

		trainModel(graph, std::make_shared<TrainSet>(graph.getHiddenSetSamples(1000, 500)), std::make_pair("D" , 0), "A");
	}, "Medium size tunable and non tunable factors model");

	EFG::sample::samplePart([]() {
		model::RandomField graph;
		xml::Importer::importFromXml(graph, SAMPLE_FOLDER , "graph_3.xml");

		trainModel(graph, std::make_shared<TrainSet>(graph.getHiddenSetSamples(500, 500)), std::make_pair("v5" , 0), "v1");
	}, "Complex tunable model");

	EFG::sample::samplePart([]() {
		VariablePtr X1 = makeVariable(2, "X1");
		VariablePtr X2 = makeVariable(2, "X2");
		VariablePtr X3 = makeVariable(2, "X3");
		VariablePtr Y1 = makeVariable(2, "Y1");
		VariablePtr Y2 = makeVariable(2, "Y2");
		VariablePtr Y3 = makeVariable(2, "Y3");

		float alfa = 2.f;
		float beta = 1.f;

		model::RandomField graph;
		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(Y1, X1), true) , beta));
		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(Y2, X2), true) , 1.f), Group(Y1,X1));  // the same weight of X1-Y1 is assumed
		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(Y3, X3), true) , 1.f), Group(Y1,X1));  // the same weight of X1-Y1 is assumed

		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(Y1, Y2), true) , alfa));
		graph.Insert(factor::modif::FactorExponential(factor::cnst::Factor(categoric::Group(Y2, X3), true) , 1.f), Group(Y1,Y2));  // the same weight of Y1-Y2 is assumed

		trainModel(graph, std::make_shared<TrainSet>(graph.getHiddenSetSamples(1000, 500)), std::make_pair("X1" , 0), "Y2");
		
	}, "Model with shared weights");

	return EXIT_SUCCESS;
}

void trainModel(model::RandomField& graph, TrainSetPtr trainSet, const std::pair<string, size_t>& checkObservation, const string& checkVariable) {
	//build a second graph, with the same potentials, but all weights equal to 1. Then use the train set made by the previous samples to train 
	//this model, for obtaining a combination of weights similar to the original one
	model::RandomField graph2Learn;
	graph2Learn.absorbStructure(static_cast<const nodes::StructureAware&>(graph), true);
	graph2Learn.absorbStructureTunable(static_cast<const nodes::StructureTunableAware&>(graph), true);
	graph2Learn.setOnes();

	GradientDescend trainer;
	trainer.setAdvancement(0.2f);

	cout << "\n\n\n real weights of the model\n";
	cout << graph.getWeights() << endl;

	cout << "\n\n\n learnt weights\n";
	cout << graph2Learn.getWeights() << endl;

	//compare the marginals distributions of the real model and the learnt one
	cout << "P(" << checkVariable << "|" << checkObservation.first << "=" << checkObservation.second << ")\n";

	cout << "real model " << endl;
	graph.resetEvidences(map<string, size_t>{ {checkObservation.first, checkObservation.second}});
	cout << graph.getMarginalDistribution(checkVariable) << endl << endl;

	cout << "learnt model " << endl;
	graph2Learn.resetEvidences(map<string, size_t>{ {checkObservation.first, checkObservation.second}});
	cout << graph2Learn.getMarginalDistribution(checkVariable) << endl << endl;
}
