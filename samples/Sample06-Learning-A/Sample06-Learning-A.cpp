/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

// what is required from the EFG core library
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/SimpleCorrelations.h>
#include <EasyFactorGraph/model/RandomField.h>

using namespace EFG::model;
using namespace EFG::factor;
using namespace EFG::categoric;
using namespace EFG::structure;

// just a bunch of utilities needed by the sample
#include <Printing.h>
#include <SampleSection.h>

#include <iostream>
using namespace std;

/// Extracts samples using Gibbs sampler to generate a training set.
/// Then, set all weights of the model to 1 and try to tune the model with the
/// previously generated train set.
/// The obtained weights are expected to be close to the initial ones.
void train_model(RandomField &model_to_tune, std::size_t train_set_size);

void add_to_model(ModelBuilder &builder, float w, std::size_t var_a,
                  std::size_t var_b, VarStateSize var_size) {
  auto factor = FactorExponential<2>::from_sparse_domain_gen(
      {var_size, var_size}, SimplyCorrelatedDomainGen<2>{var_size});
  factor.trsfm.setWeight(w);
  builder.add_tunable_binary_factor(std::move(factor), var_a, var_b);
}

int main() {
  SAMPLE_SECTION("Simple tunable model ", "4.6.1", [] {
    ModelBuilder builder;
    auto A = builder.make_variable(2);
    auto B = builder.make_variable(2);
    auto C = builder.make_variable(2);

    float alfa = 1.f, beta = 1.5f, gamma = 0.5f;

    add_to_model(builder, alfa, A, B, 2);
    add_to_model(builder, beta, A, C, 2);
    add_to_model(builder, gamma, B, C, 2);

    RandomField model{ModelBuilder::build(std::move(builder))};

    train_model(model, 500);
  });

  SAMPLE_SECTION("Medium tunable model ", "4.6.2", [] {
    ModelBuilder builder;
    auto A = builder.make_variable(2);
    auto B = builder.make_variable(2);
    auto C = builder.make_variable(2);
    auto D = builder.make_variable(2);
    auto E = builder.make_variable(2);

    float alfa = 0.4f, beta = 0.7f, gamma = 0.3f, delta = 1.5f;

    add_to_model(builder, alfa, A, B, 2);
    add_to_model(builder, beta, A, C, 2);
    add_to_model(builder, delta, D, E, 2);

    auto const_factor = BinaryFactor::from_sparse_domain_gen(
        {2, 2}, SimplyCorrelatedDomainGen<2>{2});
    builder.add_binary_factor(std::move(const_factor), C, D);

    auto const_exp_factor = BinaryFactorExponential::from_sparse_domain_gen(
        {2, 2}, SimplyCorrelatedDomainGen<2>{2});
    builder.add_binary_factor(std::move(const_exp_factor), B, E);

    RandomField model{ModelBuilder::build(std::move(builder))};

    train_model(model, 1500);
  });

  SAMPLE_SECTION("Complex tunable model ", "4.6.3", [] {
    auto path = std::filesystem::path{SAMPLE_FOLDER} / "graph_3.json";
    RandomField model{std::move(from_file(path))};

    // snap the marginal distribution of a variable before training
    auto v1 = model.getStructure().named_vars_table.at("v1");
    auto v8 = model.getStructure().named_vars_table.at("v8");
    std::vector<float> marginals_pre;
    model.setEvidences(Evidence{v1, 1});
    model.getMarginalDistribution(marginals_pre, v8);
    model.removeAllEvidences();

    train_model(model, 2000);

    // get the marginal distribution of a variable after training ... the
    // distribution shall be close
    std::vector<float> marginals_post;
    model.setEvidences(Evidence{v1, 1});
    model.getMarginalDistribution(marginals_post, v8);

    std::cout << "marginals of v8 pre tuning: " << marginals_pre << endl;
    std::cout << "marginals of v8 post tuning: " << marginals_post << endl;
  });

  return EXIT_SUCCESS;
}

void train_model(RandomField &model_to_tune, std::size_t train_set_size) {
  std::vector<float> expected_weights;
  model_to_tune.getTunableWeights(expected_weights);

  // generate the training set from the current model
  auto samples = std::make_shared<EFG::misc::Samples>(model_to_tune.makeSamples(
      GibbsSampler::SamplesGenerationContext{train_set_size, 50, 0}));

  // set all weights to 1 and train the model on the previously generated
  // train set
  std::vector<float> weights;
  weights.resize(1.f, expected_weights.size());
  EFG::structure::Trainer{}.train_model(model_to_tune, samples);

  cout << "expected weights:    " << expected_weights << endl;
  model_to_tune.getTunableWeights(weights);
  cout << "weights after train: " << weights << endl;
}
