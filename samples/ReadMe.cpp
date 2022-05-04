#include <EasyFactorGraph/distribution/Factor.h>
#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/model/RandomField.h>

int main() {
  {
      // FACTORS CONSTRUCTION
  }

  {
    // MODELS CONSTRUCTION
  }

  // define a couple of variables, all with size = 3
  EFG::categoric::VariablePtr A = EFG::categoric::make_variable(3, "A");
  EFG::categoric::VariablePtr B = EFG::categoric::make_variable(3, "B");

  // build an initial empty random field
  EFG::model::RandomField model;

  // add to the model a simply correlating factor involving A and B
  model.addConstFactor(std::make_shared<EFG::distribution::Factor>(
      EFG::categoric::Group{EFG::categoric::VariablesSoup{A, B}},
      EFG::distribution::USE_SIMPLE_CORRELATION_TAG));

  // create one additional variables
  EFG::categoric::VariablePtr C = EFG::categoric::make_variable(3, "C");

  // add some 2 more correlating factors.
  // The first one is another simply correlating factor, which is this time
  // created externally and then deep copied inside the model
  EFG::distribution::Factor factor_BC = EFG::distribution::Factor{
      EFG::categoric::Group{EFG::categoric::VariablesSoup{B, C}},
      EFG::distribution::USE_SIMPLE_CORRELATION_TAG};
  model.copyConstFactor(factor_BC);

  // The second factor we want to add to the model is a tunable one, i.e. an
  // exponential factor whose weight can be tuned.
  // The values in the domain of this factor are taken from another simply
  // correlating factor.
  EFG::distribution::Factor factor_AC = EFG::distribution::Factor{
      EFG::categoric::Group{EFG::categoric::VariablesSoup{A, C}},
      EFG::distribution::USE_SIMPLE_CORRELATION_TAG};
  const float weight_AC = 1.3f;
  model.addTunableFactor(std::make_shared<EFG::distribution::FactorExponential>(
      factor_AC, weight_AC));

  return EXIT_SUCCESS;
}
