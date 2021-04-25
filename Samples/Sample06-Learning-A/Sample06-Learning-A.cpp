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
#include <CombinationMaker.h>
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

std::vector<Combination> getGibbsSamples(model::RandomField& graph, std::size_t numberOfSamples, std::size_t deltaIteration);

void trainModel(model::RandomField& graph, TrainSetPtr trainSet, train::Trainer& trainer, const std::pair<string, size_t>& checkObservation, const string& checkVariable , const std::size_t& threads = 1);

int main() {	
	EFG::sample::samplePart([]() {
		//build the structure explained in 'Sample 06: Learning, part A / part 01'
		VariablePtr A = makeVariable(2, "A");
		VariablePtr B = makeVariable(2, "B");
		VariablePtr C = makeVariable(2, "C");

		float alfa = 1.f, beta = 3.f, gamma = 0.1f;

		model::RandomField graph;
		graph.insertCopy(factor::cnst::FactorExponential(factor::cnst::Factor({ A, B }, true), alfa)); // the weight of this potential will be kept constant
		graph.insertTunable(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor({ A, C }, true), beta));
		graph.insertTunable(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor({ B, C }, true), gamma));

		//extract some samples form the joint distributions of the variable in the graph, using a Gibbs sampling method
		auto samples = getGibbsSamples(graph, 500, 100);

		float Z = 2.f*(expf(alfa) + expf(beta) + expf(gamma) + expf(alfa)*expf(beta)*expf(gamma));
		auto hidden_set = graph.getHiddenVariables();

		cout << "freq <0,0,0> " << endl;
		cout << "theoretical " << expf(alfa) * expf(beta) * expf(gamma) / Z << endl;
		cout << "Gibbs sampler results " << sample::getEmpiricalFrequencies(sample::makeCombination({ 0,0,0 }), Group(A, B, C), samples, Group(A, B, C).getVariables()) << endl << endl;

		cout << "freq <1,0,0> " << endl;
		cout << "theoretical " << expf(gamma) / Z << endl;
		cout << "Gibbs sampler results " << sample::getEmpiricalFrequencies(sample::makeCombination({ 1,0,0 }), Group(A, B, C), samples, Group(A, B, C).getVariables()) << endl << endl;

		cout << "freq <0,1,0> " << endl;
		cout << "theoretical " << expf(beta) / Z << endl;
		cout << "Gibbs sampler results " << sample::getEmpiricalFrequencies(sample::makeCombination({ 0,1,0 }), Group(A, B, C), samples, Group(A, B, C).getVariables()) << endl << endl;

		cout << "freq <1,1,0> " << endl;
		cout << "theoretical " << expf(alfa) / Z << endl;
		cout << "Gibbs sampler results " << sample::getEmpiricalFrequencies(sample::makeCombination({ 1,1,0 }), Group(A, B, C), samples, Group(A, B, C).getVariables()) << endl << endl;

		GradientDescend trainer;
		trainer.setMaxIterations(50);
		trainModel(graph, std::make_shared<TrainSet>(samples), trainer, std::make_pair("C" , 0), "A");
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
		graph.insertCopy(factor::cnst::FactorExponential(factor::cnst::Factor({ A, B }, true), alfa)); // the weight of this potential will be kept constant
		graph.insertTunableCopy(factor::modif::FactorExponential(factor::cnst::Factor({ A, C }, true), beta));
		graph.insertCopy(factor::cnst::Factor({ B, C }, true));
		graph.insertCopy(factor::cnst::FactorExponential(factor::cnst::Factor({ B, E }, true), gamma)); // the weight of this potential will be kept constant
		graph.insertTunableCopy(factor::modif::FactorExponential(factor::cnst::Factor({ B, D }, true), delta));
		graph.insertCopy(factor::cnst::Factor({ D, E }, true));

		GradientDescend trainer;
		trainer.setAdvancement(0.1f);
		trainModel(graph, std::make_shared<TrainSet>(getGibbsSamples(graph, 1000, 100)), trainer, std::make_pair("D" , 0), "A");
	}, "Medium size tunable and non tunable factors model");

	EFG::sample::samplePart([]() {
		model::RandomField graph;
		xml::Importer::importFromXml(graph, SAMPLE_FOLDER , "graph_3.xml");

		GradientDescend trainer;
		trainer.setAdvancement(0.1f);
		trainModel(graph, std::make_shared<TrainSet>(getGibbsSamples(graph, 1500, 100)), trainer, std::make_pair("v5" , 0), "v1", 3);
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
		graph.insertTunableCopy(factor::modif::FactorExponential(factor::cnst::Factor({ Y1, X1 }, true), beta));
		graph.insertTunableCopy(factor::modif::FactorExponential(factor::cnst::Factor({ Y2, X2 }, true), 1.f), { Y1, X1 });  // the same weight of X1-Y1 is assumed
		graph.insertTunableCopy(factor::modif::FactorExponential(factor::cnst::Factor({ Y3, X3 }, true), 1.f), { Y1, X1});  // the same weight of X1-Y1 is assumed

		graph.insertTunableCopy(factor::modif::FactorExponential(factor::cnst::Factor({ Y1, Y2 }, true), alfa));
		graph.insertTunableCopy(factor::modif::FactorExponential(factor::cnst::Factor({ Y2, X3 }, true), 1.f), { Y1, Y2 });  // the same weight of Y1-Y2 is assumed

		GradientDescend trainer;
		trainer.setMaxIterations(50);
		trainModel(graph, std::make_shared<TrainSet>(getGibbsSamples(graph, 1000, 100)), trainer, std::make_pair("X1" , 0), "Y2");
	}, "Model with shared weights");

	return EXIT_SUCCESS;
}

std::vector<Combination> getGibbsSamples(model::RandomField& graph, std::size_t numberOfSamples, std::size_t deltaIteration) {
#ifdef THREAD_POOL_ENABLED
	graph.setThreadPoolSize(4);
#endif
	auto samples = graph.getHiddenSetSamples(numberOfSamples, deltaIteration);
#ifdef THREAD_POOL_ENABLED
	graph.setThreadPoolSize(0);
#endif
	return samples;
}

void trainModel(model::RandomField& graph, TrainSetPtr trainSet, train::Trainer& trainer, const std::pair<string, size_t>& checkObservation, const string& checkVariable, const std::size_t& threads) {
	//build a second graph, with the same potentials, but all weights equal to 1. Then use the train set made by the previous samples to train 
	//this model, for obtaining a combination of weights similar to the original one
	model::RandomField graph2Learn;
	graph2Learn.absorbModel(graph, true);
	graph2Learn.setOnes();
#ifdef THREAD_POOL_ENABLED
	if (threads > 1) {
		graph2Learn.setThreadPoolSize(threads);
	}
#endif

	trainer.train(graph2Learn, trainSet);

	cout << "\n real weights of the model\n";
	cout << graph.getWeights() << endl;

	cout << "learnt weights\n";
	cout << graph2Learn.getWeights() << endl;

	//compare the marginals distributions of the real model and the learnt one
	cout << "P(" << checkVariable << '|' << checkObservation.first << '=' << checkObservation.second << ")\n";

	cout << "real model " << endl;
	graph.resetEvidences(map<string, size_t>{ {checkObservation.first, checkObservation.second}});
	cout << graph.getMarginalDistribution(checkVariable) << endl << endl;

	cout << "learnt model " << endl;
	graph2Learn.resetEvidences(map<string, size_t>{ {checkObservation.first, checkObservation.second}});
	cout << graph2Learn.getMarginalDistribution(checkVariable) << endl << endl;
}
