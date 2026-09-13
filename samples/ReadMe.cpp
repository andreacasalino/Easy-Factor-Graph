#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/model/ConditionalRandomField.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/SamplesImport.h>

#include <filesystem>

int main() {
  {
    // FACTORS CONSTRUCTION

    // Factors are just shape functions.
    // For examample this is an exponential simply correlating factor connecting
    // 2 variables each with space size equal to 3
    float weight = 1.5f;
    EFG::factor::FactorExponential<2> factor_exp =
        EFG::factor::make_exp_simply_correlated<2>(3, weight);

    // This other factor is instead built defining value by value the element in
    // its domain
    //
    // This factor will connet 2 variables:
    // - the first having a space size of 3
    // - the second having a space size of 2
    //
    // domain is us:
    // - set for <0,1> -> 2
    // - set for <2,0> -> 1.3f
    EFG::factor::Factor<2> factor =
        EFG::factor::Factor<2>::fromSparseDomain<true>(
            EFG::categoric::Combination<2>{3, 2},
            {{{0, 1}, 2.f}, {{2, 0}, 1.3f}});
  }

  {
    // MODELS CONSTRUCTION

    // builder pattern to define the pieces of the model
    EFG::structure::ModelBuilder builder;

    // define some variables, which will be later connected
    EFG::categoric::VarStateSize space_size{4};
    auto varA_idx = builder.make_variable(space_size);
    auto varB_idx = builder.make_variable(space_size);
    auto varC_idx = builder.make_variable(space_size);
    auto varD_idx = builder.make_variable(space_size);

    // add constant factors
    builder.add_binary_factor(
        EFG::factor::make_exp_simply_correlated<2>(space_size, 1.2f), varA_idx,
        varB_idx);

    // add additional tunable factors
    builder.add_binary_factor(
        EFG::factor::make_exp_simply_correlated<2>(space_size, 0.7f), varB_idx,
        varC_idx);
    builder.add_binary_factor(
        EFG::factor::make_exp_simply_correlated<2>(space_size, 1.5f), varC_idx,
        varD_idx);

    // now seal the model and actually build it
    EFG::model::RandomField model{
        EFG::structure::ModelBuilder::build(std::move(builder))};

    // export the model to a file
    std::filesystem::path dest_file = "/tmp/the_mdoel.json";
    model.to_file(dest_file);
  }

  {
    // QUERY THE MODEL

    EFG::structure::ModelBuilder builder;
    EFG::model::RandomField model{
        EFG::structure::ModelBuilder::build(std::move(builder))};

    // set some evidences
    auto varA_idx = model.getStructure().named_vars_table.at("varA");
    auto varB_idx = model.getStructure().named_vars_table.at("varB");
    EFG::structure::Evidence ev_A{varA_idx, 1}; // prescribe setting varA = 1
    EFG::structure::Evidence ev_B{varB_idx, 0}; // prescribe setting varB = 0
    model.setEvidences(ev_A, ev_B);

    // get the marginal conditioned distribution of an hidden variable
    std::vector<float> conditioned_marginals;
    auto varC_idx = model.getStructure().named_vars_table.at(
        "varC"); // there is no need to look up every time the variable if you
                 // already know the index
    model.getMarginalDistribution(conditioned_marginals, varC_idx);

    // get maxiomum a posteriori estimation of the entire hidden set
    std::vector<EFG::categoric::VarStateSize> MAP_hidden_set;
    model.getHiddenSetMAP(
        MAP_hidden_set); // same order of variable accessible through
                         // model.getStructure().nodes, keeping only hidden ones
                         // actually, is assumed

    // set some new evidences
    model.removeAllEvidences();
    model.setEvidences(EFG::structure::Evidence{
        model.getStructure().named_vars_table.at("varE"), 1});

    // compute new conditioned marginals: the should be different as the
    // evidences were changed
    model.getMarginalDistribution(conditioned_marginals, varA_idx);
  }

  {
    // TUNE THE MODEL

    // assume we have a training set for the model stored in a file
    auto training_set = std::make_shared<EFG::misc::Samples>(
        EFG::structure::load_train_set("/tmp/the_training_set", 500));

    EFG::structure::ModelBuilder builder;
    EFG::model::ConditionalRandomField tunable_model{
        EFG::structure::from_file("/tmp/the_model.json")};

    // We can train the model using the function provided by this library, which
    // is a simple gradient descend
    //
    // At the same what this library offers is the possibility to get/set the
    // weights of the model as well as evaluate their gradient w.r.t. a given
    // training set. Once you have this you can implement any traning startegy!
    EFG::structure::Trainer{}
        .max_iterations(500)
        .gradient_rescale(0.02f)
        .train_model(tunable_model, training_set);
  }

  {
    // GIBBS SAMPLING

    EFG::model::RandomField model{
        EFG::structure::from_file("/tmp/the_model.json")};

    // some definitions to control the samples generation process
    EFG::structure::GibbsSampler::SamplesGenerationContext samples_gen_context{
        .samples_number = 1000, // samples number
        .seed = 0,              // seed used by random engines
    };

    // get samples from the model using Gibbs sampler
    EFG::misc::Samples samples = model.makeSamples(samples_gen_context);
  }

  return EXIT_SUCCESS;
}
