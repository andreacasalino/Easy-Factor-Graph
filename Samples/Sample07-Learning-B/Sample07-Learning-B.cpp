/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

 // This example is also explained in the documentation (the .pdf file), Section 'Learning of graphs, part B'

#include <model/ConditionalRandomField.h>
#include <train/trainers/GradientDescend.h>
#include <categoric/Range.h>
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

TrainSetPtr makeCondModelTrainSet(model::ConditionalRandomField& Model);

int main() {
	EFG::sample::samplePart([]() {
		model::ConditionalRandomField graph(std::string(SAMPLE_FOLDER), std::string("cond_graph.xml"));

		GradientDescend<StochasticExtractor> trainer;
		trainer.setAdvancement(0.1f);
		//trainer.setPercentage(0.1f);

		TrainSetPtr trainSet = makeCondModelTrainSet(graph);

		//build a second graph, with the same potentials, but all weights equal to 1. Then use the train set made by the previous samples to train 
		//this model, for obtaining a combination of weights similar to the original one
		model::ConditionalRandomField graph2Learn(graph);
		graph2Learn.setOnes();
#ifdef THREAD_POOL_ENABLED
		graph2Learn.SetThreadPoolSize(3);
#endif

		trainer.train(graph2Learn, trainSet);

		// compare the marginals computation of the real and the learnt models
		// adopting as observations the values {0,1,0,1,...}
		size_t O = graph2Learn.getEvidences().size();
		vector<size_t> obs;
		obs.reserve(O);
		bool temp = false;
		for (size_t k = 0; k < O; k++) {
			obs.push_back((size_t)temp);
			temp = !temp;
		}

		cout << "\n real weights of the model\n";
		cout << graph.getWeights() << endl;

		cout << "learnt weights\n";
		cout << graph2Learn.getWeights() << endl;

		//compare the marginals distributions of the real model and the learnt one
		cout << "P(Y4|X={0,1,0,1,...}" << endl;

		cout << "real model " << endl;
		graph.setEvidences(obs);
		cout << graph.getMarginalDistribution("Y4") << endl << endl;

		cout << "learnt model " << endl;
		graph2Learn.setEvidences(obs);
		cout << graph2Learn.getMarginalDistribution("Y4") << endl << endl;
	}, "Learning of conditioned model");

	return EXIT_SUCCESS;
}

TrainSetPtr makeCondModelTrainSet(model::ConditionalRandomField& Model) {
	auto itO = Model.getEvidences().begin();
	categoric::Group groupObs(itO->first);
	++itO;
	for (itO; itO != Model.getEvidences().end(); ++itO) {
		groupObs.add(itO->first);
	}
	categoric::Range jointDomainObs(groupObs);
	std::size_t deltaSamples = 15;
	vector<Combination> samples;
	samples.reserve(deltaSamples * groupObs.size());

	std::vector<std::size_t> posHidden, posObsv;
	auto allVars = Model.getVariables();

	auto hiddenVars = Model.getHiddenVariables();
	posHidden.reserve(hiddenVars.size());
	for (auto itH = hiddenVars.begin(); itH != hiddenVars.end(); ++itH) {
		posHidden.push_back(std::distance(allVars.begin(), allVars.find(*itH)));
	}

	posObsv.reserve(groupObs.getVariables().size());
	for (auto itO = groupObs.getVariables().begin(); itO != groupObs.getVariables().end(); ++itO) {
		posObsv.push_back(std::distance(allVars.begin(), allVars.find(*itO)));
	}

	EFG::iterator::forEach(jointDomainObs, [&](const categoric::Range& range) {
		auto obs = range.get();
		std::vector<std::size_t> combination(posHidden.size() + posObsv.size(), 0);
		for (std::size_t k = 0; k < posObsv.size(); ++k) {
			combination[posObsv[k]] = obs[k];
		}
		Model.setEvidences(obs);
		auto samplesNew = Model.getHiddenSetSamples(deltaSamples, 100);
		for (auto itS = samplesNew.begin(); itS != samplesNew.end(); ++itS) {
			for (std::size_t k = 0; k < posHidden.size(); ++k) {
				combination[posHidden[k]] = itS->data()[k];
			}
			samples.emplace_back(combination);
		}
	});
	return std::make_shared<TrainSet>(samples);
}
