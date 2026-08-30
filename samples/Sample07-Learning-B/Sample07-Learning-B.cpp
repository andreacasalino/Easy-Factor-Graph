/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/SimpleCorrelations.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>

using namespace EFG::model;
using namespace EFG::factor;
using namespace EFG::categoric;
using namespace EFG::structure;

// you can also use another iterative trainer
#include <TrainingTools/iterative/solvers/QuasiNewton.h>

using namespace train;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  SAMPLE_SECTION("Tuning of a conditional random field ", "4.7", [] {
    auto path = std::filesystem::path{SAMPLE_FOLDER} / "cond_graph.json";
    ConditionalRandomField model{std::move(from_file(path))};

    cout << "creating the training set, might take a while" << endl;
    auto samples = std::make_shared<EFG::misc::Samples>(
        model.makeSamples(GibbsSampler::SamplesGenerationContext{1000, 50, 0}));
    cout << "training set created" << endl;

    std::vector<float> expected_weights;
    model.getTunableWeights(expected_weights);

    // set all weights to 1 and train the model on the previously generated
    // train set
    std::vector<float> weights;
    weights.resize(1.f, expected_weights.size());
    model.setTunableWeights(weights);

    QuasiNewton trainer;
    trainer.setMaxIterations(100);
    cout << "training the model, this might take a while as conditional "
            "random "
            "field are much more computationally demanding"
         << endl;
    trainer.enablePrintAdvancement();
    EFG::structure::train_model(model, trainer, samples);

    cout << "expected weights:    " << expected_weights << endl;
    model.getTunableWeights(weights);
    cout << "weights after train: " << weights << endl;
  });

  return EXIT_SUCCESS;
}
