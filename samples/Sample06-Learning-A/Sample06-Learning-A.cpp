/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/trainable/ModelTrainer.h>

using namespace EFG::model;
using namespace EFG::factor;
using namespace EFG::categoric;
using namespace EFG::strct;
using namespace EFG::io;
using namespace EFG::train;

// you can also use another iterative trainer
#include <TrainingTools/iterative/solvers/GradientDescend.h>
#include <TrainingTools/iterative/solvers/QuasiNewton.h>

using namespace train;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

/// Extracts samples using Gibbs sampler to generate a training set.
/// Then, set all weights of the model to 1 and try to tune the model with the
/// previously generated train set.
/// The obtained weights are expected to be close to the initial ones.
void train_model(RandomField &model_to_tune, ::train::IterativeTrainer &tuner,
                 const std::size_t max_iterations,
                 const std::size_t train_set_size);

int main() {
  SAMPLE_SECTION("Simple tunable model ", "4.6.1", [] {
    RandomField model;

    VariablePtr A = make_variable(2, "A");
    VariablePtr B = make_variable(2, "B");
    VariablePtr C = make_variable(2, "C");

    float alfa = 1.f, beta = 1.5f, gamma = 0.5f;

    model.addTunableFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{A, B}, Factor::SimplyCorrelatedTag{}}, alfa));
    model.addTunableFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{A, C}, Factor::SimplyCorrelatedTag{}}, beta));
    model.addTunableFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{B, C}, Factor::SimplyCorrelatedTag{}}, gamma));

    ::train::QuasiNewton tuner;
    train_model(model, tuner, 50, 500);
  });

  SAMPLE_SECTION("Medium tunable model ", "4.6.2", [] {
    RandomField model;

    VariablePtr A = make_variable(2, "A");
    VariablePtr B = make_variable(2, "B");
    VariablePtr C = make_variable(2, "C");
    VariablePtr D = make_variable(2, "D");
    VariablePtr E = make_variable(2, "E");

    float alfa = 0.4f, beta = 0.7f, gamma = 0.3f, delta = 1.5f;

    model.addConstFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{A, B}, Factor::SimplyCorrelatedTag{}},
        alfa)); // the weight of this potential will be
                // kept constant
    model.addTunableFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{A, C}, Factor::SimplyCorrelatedTag{}}, beta));
    model.addConstFactor(std::make_shared<Factor>(
        VariablesSoup{C, D}, Factor::SimplyCorrelatedTag{}));
    model.addConstFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{B, E}, Factor::SimplyCorrelatedTag{}},
        gamma)); // the weight of this potential will be
                 // kept constant
    model.addTunableFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{D, E}, Factor::SimplyCorrelatedTag{}}, delta));

    ::train::QuasiNewton tuner;
    train_model(model, tuner, 50, 1500);
  });

  SAMPLE_SECTION("Complex tunable model ", "4.6.3", [] {
    RandomField model;
    xml::Importer::importFromFile(model,
                                  SAMPLE_FOLDER + std::string{"graph_3.xml"});

    ::train::QuasiNewton tuner;
    train_model(model, tuner, 50, 2000);
  });

  SAMPLE_SECTION("Model with sharing weights", "4.6.4", [] {
    RandomField model;

    VariablePtr X1 = make_variable(2, "X1");
    VariablePtr X2 = make_variable(2, "X2");
    VariablePtr X3 = make_variable(2, "X3");

    VariablePtr Y1 = make_variable(2, "Y1");
    VariablePtr Y2 = make_variable(2, "Y2");
    VariablePtr Y3 = make_variable(2, "Y3");

    float alfa = 2.f;
    float beta = 1.f;

    model.addTunableFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{Y1, Y2}, Factor::SimplyCorrelatedTag{}}, alfa));
    model.addTunableFactor(
        std::make_shared<FactorExponential>(
            Factor{VariablesSoup{Y2, Y3}, Factor::SimplyCorrelatedTag{}}, alfa),
        VariablesSet{Y1, Y2}); // this will force this added factor to share the
                               // weight with the one connecting variables Y1,Y2

    model.addTunableFactor(std::make_shared<FactorExponential>(
        Factor{VariablesSoup{Y1, X1}, Factor::SimplyCorrelatedTag{}}, beta));
    model.addTunableFactor(
        std::make_shared<FactorExponential>(
            Factor{VariablesSoup{Y2, X2}, Factor::SimplyCorrelatedTag{}}, beta),
        VariablesSet{Y1, X1}); // this will force this added factor to share the
                               // weight with the one connecting variables Y1,X1
    model.addTunableFactor(
        std::make_shared<FactorExponential>(
            Factor{VariablesSoup{Y3, X3}, Factor::SimplyCorrelatedTag{}}, beta),
        VariablesSet{Y1, X1}); // this will force this added factor to share the
                               // weight with the one connecting variables Y1,X1

    ::train::QuasiNewton tuner;
    train_model(model, tuner, 50, 1000);
  });

  return EXIT_SUCCESS;
}

void train_model(RandomField &model_to_tune, ::train::IterativeTrainer &tuner,
                 const std::size_t max_iterations,
                 const std::size_t train_set_size) {
  const auto expected_weights = model_to_tune.getWeights();

  // generate the training set from the current model
  auto samples = model_to_tune.makeSamples(
      GibbsSampler::SamplesGenerationContext{train_set_size, 50, 0});

  // set all weights to 1 and train the model on the previously generated
  // train set
  set_ones(model_to_tune);
  tuner.setMaxIterations(max_iterations);
  train_model(model_to_tune, tuner, TrainSet{samples});

  cout << "expected weights:    " << expected_weights << endl;
  cout << "weights after train: " << model_to_tune.getWeights() << endl;
}
