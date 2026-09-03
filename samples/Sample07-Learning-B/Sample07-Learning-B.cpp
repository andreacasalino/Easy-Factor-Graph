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

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

int main() {
  SAMPLE_SECTION("Tuning of a conditional random field ", "4.7", [] {
    auto path = std::filesystem::path{SAMPLE_FOLDER} / "cond_graph.json";
    ConditionalRandomField model{std::move(from_file(path))};

    // ModelBuilder builder;
    // auto X1 = builder.make_variable(
    //     2, ModelBuilder::VariableDescription{.isPermanentEvidence = true});
    // auto X2 = builder.make_variable(
    //     2, ModelBuilder::VariableDescription{.isPermanentEvidence = true});
    // auto Y1 = builder.make_variable(2);
    // auto Y2 = builder.make_variable(2);
    // auto Y3 = builder.make_variable(2);
    // builder.add_tunable_binary_factor(make_exp_simply_correlated<2>(2, 1.5f),
    //                                   X1, Y1);
    // builder.add_tunable_binary_factor(make_exp_simply_correlated<2>(2, 1.f),
    // X2,
    //                                   Y2);
    // builder.add_tunable_binary_factor(make_exp_simply_correlated<2>(2, 0.2f),
    //                                   Y1, Y2);
    // builder.add_tunable_binary_factor(make_exp_simply_correlated<2>(2, 0.8f),
    //                                   Y1, Y3);
    // builder.add_tunable_binary_factor(make_exp_simply_correlated<2>(2, 1.2f),
    //                                   Y2, Y3);
    // ConditionalRandomField model{ModelBuilder::build(std::move(builder))};

    cout << "creating the training set, might take a while" << endl;
    auto samples =
        std::make_shared<EFG::misc::Samples>(model.makeSamplesSpanningEvidences(
            GibbsSampler::SamplesGenerationContext{2000, 50, 0, true}));
    cout << "training set created" << endl;

    std::vector<float> expected_weights;
    model.getTunableWeights(expected_weights);

    // set all weights to 1 and train the model on the previously generated
    // train set
    std::vector<float> weights;
    weights.resize(expected_weights.size(), 1.f);
    model.setTunableWeights(weights);

    cout << "training the model, this might take a while as conditional "
            "random "
            "field are much more computationally demanding"
         << endl;
    // use more threads to speed up the process
    model.setWorkersPoolSize(4);
    {
      // auto scoped_activation = model.activatePool();
      EFG::structure::Trainer{}.max_iterations(50).train_model_with_cb(
          model, samples, [i = int{0}](const std::vector<float> &) mutable {
            cout << "iteration: " << ++i << " done" << endl;
          });
    }

    cout << "expected weights:    " << expected_weights << endl;
    model.getTunableWeights(weights);
    cout << "weights after train: " << weights << endl;
  });

  return EXIT_SUCCESS;
}
