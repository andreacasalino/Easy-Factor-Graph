// #include <catch2/catch_test_macros.hpp>
// #include <catch2/generators/catch_generators.hpp>

// #include <EasyFactorGraph/model/RandomField.h>

// using namespace EFG;
// using namespace EFG::categoric;
// using namespace EFG::distribution;
// using namespace EFG::model;
// using namespace EFG::train;
// using namespace EFG::strct;

// TEST_CASE("Gradient evaluation on binary factor", "[train][gradient]") {
//   RandomField model;

//   const float w = 1.5f;
//   FactorExponential factor_AB(
//       Factor{Group{VariablesSoup{make_variable(2, "A"), make_variable(2,
//       "B")}},
//              USE_SIMPLE_CORRELATION_TAG},
//       w);
//   model.copyTunableFactor(factor_AB);

//   auto samples = model.getHiddenSetSamples(
//       GibbsSampler::SamplesGenerationContext{100, 20, 0});
// }
