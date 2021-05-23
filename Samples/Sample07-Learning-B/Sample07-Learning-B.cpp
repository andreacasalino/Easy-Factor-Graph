/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
*
* report any bug to andrecasa91@gmail.com.
 **/

#include <model/ConditionalRandomField.h>
#include <train/trainers/GradientDescend.h>
#include <categoric/Range.h>
#include <io/xml/Importer.h>
#include <print/ProbabilityDistributionPrint.h>
#include <Presenter.h>
#include <Frequencies.h>
#include <TrainSetCreator.h>
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
		model::ConditionalRandomField graph(EFG::io::FilePath(SAMPLE_FOLDER, "cond_graph.xml"));

		cout << "creating the training set, might take a while" << endl;
		TrainSetPtr trainSet = makeCondModelTrainSet(graph);
		cout << "training set created" << endl;

		//build a second graph, with the same potentials, but all weights equal to 1. Then use the train set made by the previous samples to train 
		//this model, for obtaining a combination of weights similar to the original one
		model::ConditionalRandomField graph2Learn(graph);
		graph2Learn.setOnes();
#ifdef THREAD_POOL_ENABLED
		graph2Learn.setThreadPoolSize(4);
#endif

		// use stochastic gradient descend
		GradientDescend<StochasticExtractor> trainer;
		trainer.setPercentage(0.05f);
		trainer.setMaxIterations(10);
		cout << "training the model, take some time cause conditioned model are much more computational demanding ..." << endl;
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
		cout << graph2Learn.getWeights() << endl << endl;

		//compare the marginals distributions of the real model and the learnt one
		cout << "P(Y4|X={0,1,0,1,...}" << endl;

		cout << "real model " << endl;
		graph.setEvidences(obs);
		cout << graph.getMarginalDistribution("Y4") << endl << endl;

		cout << "learnt model " << endl;
		graph2Learn.setEvidences(obs);
		cout << graph2Learn.getMarginalDistribution("Y4") << endl << endl;
	}, "Tuning of a conditional random field", "refer to Section 4.7");

	return EXIT_SUCCESS;
}

void append(std::vector<Combination>& recipient, const std::vector<Combination>& toAdd) {
	for (auto it = toAdd.begin(); it != toAdd.end(); ++it) {
		recipient.push_back(*it);
	}
};
TrainSetPtr makeCondModelTrainSet(model::ConditionalRandomField& Model) {
	std::size_t deltaSamples = 50;

	auto itO = Model.getEvidences().begin();
	std::set<categoric::VariablePtr> groupObs = { itO->first };
	++itO;
	for (itO; itO != Model.getEvidences().end(); ++itO) {
		groupObs.emplace(itO->first);
	}

#ifdef THREAD_POOL_ENABLED
	Model.setThreadPoolSize(4);
#endif

	std::vector<Combination> trainSet;
	trainSet.reserve(groupObs.size() * deltaSamples);

	auto comb2Vector = [](const categoric::Combination& comb) {
		std::vector<std::size_t> temp;
		temp.reserve(comb.size());
		for (std::size_t k = 0; k < comb.size(); ++k) {
			temp.push_back(comb.data()[k]);
		}
		return temp;
	};

	sample::TrainSetCreator sampler(Model);
	categoric::Range rangeObs(groupObs);
	EFG::iterator::forEach(rangeObs, [&](categoric::Range& range) {
		append(trainSet, sampler.getSamples(comb2Vector(range.get()), deltaSamples, 20));
	});
	return std::make_shared<TrainSet>(trainSet);
}
