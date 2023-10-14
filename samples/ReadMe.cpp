#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/io/TrainSetImport.h>
#include <EasyFactorGraph/io/json/Importer.h>
#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/trainable/ModelTrainer.h>

#include <TrainingTools/iterative/solvers/QuasiNewton.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::factor;
using namespace EFG::model;
using namespace EFG::io;
using namespace EFG::train;
using namespace EFG::strct;

int main() {
  {
    // FACTORS CONSTRUCTION

    // define a couple of variables, with the same size
    VariablePtr A = make_variable(3, "A"); // size is 3
    VariablePtr B = make_variable(3, "B"); // size is 3

    // build a simply correlating factor connecting the 2 variables
    Factor factor_AB(VariablesSoup{B, A}, // the order in the specified
                                          // group matters: B is assumed
                                          // as the first variable, A
                                          // will be the second
                     Factor::SimplyCorrelatedTag{});

    // build an exponential factor using as base `factor_AB`: values of the
    // images are assumed as exp(weight * images_factor_AB)
    FactorExponential factor_AB_exponential(
        factor_AB,
        1.5f // this will be the value assumed for the weight
    );

    // define another variable
    VariablePtr C = make_variable(2, "C"); // size is 2
    // define a factor connecting C to B
    // we start building an empty factor, having all images equal to 0
    Factor factor_BC(VariablesSoup{B, C});
    // set some individual images of factor_BC
    // set for <0,1> -> 2
    factor_BC.set(std::vector<std::size_t>{0, 1}, 2.f);
    // set for <2,0> -> 1.3f
    factor_BC.set(std::vector<std::size_t>{2, 0}, 1.3f);
  }

  {
    // MODELS CONSTRUCTION

    // start building an empty random field
    RandomField model;

    // define some variables, which will be later connected
    auto A = make_variable(4, "varA");
    auto B = make_variable(4, "varB");
    auto C = make_variable(4, "varC");

    // without loss of generality, add to the model some simply correlating
    // factors
    model.addConstFactor(std::make_shared<Factor>(
        VariablesSoup{A, B},
        Factor::SimplyCorrelatedTag{})); // the generated smart
                                         // pointer is shallow
                                         // copied
    model.copyConstFactor(
        Factor{VariablesSoup{A, C},
               Factor::SimplyCorrelatedTag{}}); // the passed factor is
                                                // deep-copied into the
                                                // model

    // build some additional tunable exponential factors that will be too added
    auto factor_exp_BC = std::make_shared<FactorExponential>(
        Factor{VariablesSoup{B, C}, Factor::SimplyCorrelatedTag{}}, 1.f);
    model.addTunableFactor(factor_exp_BC);

    auto D = make_variable(4, "varD");
    auto factor_exp_CD = std::make_shared<FactorExponential>(
        Factor{VariablesSoup{C, D}, Factor::SimplyCorrelatedTag{}}, 1.5f);
    model.addTunableFactor(factor_exp_CD);

    // insert another tunable factor, this time specifying that it needs to
    // share the weight with already inserted exponential factor that connects B
    // and C
    model.addTunableFactor(
        std::make_shared<FactorExponential>(
            Factor{VariablesSoup{C, D}, Factor::SimplyCorrelatedTag{}},
            2.f // actually this value is irrelevant, as the weight of
                // factor_exp_BC will be assumed from now on
            ),
        VariablesSet{B, C}
        // this additional input is to specify that this exponential factor
        // needs to share the weight with the one connecting B and C
    );

    // absorb the structure defined in an xml file
    xml::Importer::importFromFile(model, std::string{"file_name.xml"});

    // absorb the structure encoded in a json string
    nlohmann::json json_defining_a_structure = ...;
    json::Importer::importFromJson(model, json_defining_a_structure);
  }

  {
    // QUERY THE MODEL

    RandomField model;

    // set some evidences
    model.setEvidence("variable_1", 0); // setting variable_1 = 0
    model.setEvidence("variable_2", 2); // setting variable_2 = 2

    // get the marginal conditioned distribution of an hidden variable
    std::vector<float> conditioned_marginals =
        model.getMarginalDistribution("var_A");

    // get maxiomum a posteriori estimation of the entire hidden set
    std::vector<std::size_t> MAP_hidden_set = model.getHiddenSetMAP();

    // set some new evidences
    model.removeAllEvidences();
    model.setEvidence("evid_1", 1);

    // compute new conditioned marginals: the should be different as the
    // evidences were changed
    conditioned_marginals = model.getMarginalDistribution("var_A");
  }

  {
    // TUNE THE MODEL

    RandomField tunable_model;

    // assume we have a training set for the model stored in a file
    TrainSet training_set = import_train_set("file_name.txt");

    // we can train the model using one of the ready to use gradient based
    // approaches
    ::train::QuasiNewton ready_to_use_trainer;
    ready_to_use_trainer.setMaxIterations(50);

    // some definitions to control the training process
    TrainInfo info = TrainInfo{
        4,  // threads to use
        1.f // stochasticity. When set different from 1, the stochastich
            // gradient descend approaches are actually used
    };

    train_model(tunable_model, ready_to_use_trainer, training_set, info);
  }

  {
    // GIBBS SAMPLING
    RandomField model;

    // some definitions to control the samples generation process
    GibbsSampler::SamplesGenerationContext info =
        GibbsSampler::SamplesGenerationContext{
            1000, // samples number
            0,    // seed used by random engines
            500   // number of iterations to discard at the beginning (burn out)
        };

    // get samples from the model using Gibbs sampler
    std::vector<std::vector<std::size_t>> samples =
        model.makeSamples(info,
                          4 // threads to use
        );
  }

  return EXIT_SUCCESS;
}
