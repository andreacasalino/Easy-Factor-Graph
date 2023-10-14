#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Utils.h"
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/trainable/tuners/BaseTuner.h>

namespace EFG::test {
using namespace categoric;
using namespace train;
using namespace strct;
using namespace model;

namespace {
float exp_over_exp_plus_one(float val) {
  float exp_val = expf(val);
  return exp_val / (1.f + exp_val);
}

class TunableModelTest : public RandomField {
public:
  TunableModelTest() = default;

  TrainSet makeGoodTrainset(std::size_t samples) const {
    return make_good_trainset(*this, samples);
  }

  bool checkGradient(const std::vector<float> &for_samples_generation,
                     const std::vector<float> &for_gradient_computation) {
    setWeights(for_samples_generation);
    auto samples = makeGoodTrainset(1000);

    setWeights(for_gradient_computation);
    propagateBelief(PropagationKind::SUM);
    auto samples_it = samples.makeIterator();
    for (std::size_t t = 0; t < tuners.size(); ++t) {
      const auto &tuner = tuners[t];

      float alfa_part = tuner->getGradientAlpha(samples_it);
      float alfa_expected = exp_over_exp_plus_one(for_samples_generation[t]);

      float beta_part = tuner->getGradientBeta();
      float beta_expected = exp_over_exp_plus_one(tuner->getWeight());

      if (!almost_equal(alfa_part, alfa_expected, 0.05f)) {
        return false;
      }
      if (!almost_equal(beta_part, beta_expected, 0.05f)) {
        return false;
      }
    }
    return true;
  }
};
} // namespace

TEST_CASE("Gradient evaluation on binary factor", "[gradient]") {
  TunableModelTest model;

  const float w = 1.f;
  model.addTunableFactor(
      make_corr_expfactor_ptr(make_variable(2, "A"), make_variable(2, "B"), w));

  auto grad_w = GENERATE(0.5f, 2.f);

  CHECK(model.checkGradient(std::vector<float>{w}, std::vector<float>{grad_w}));
}

TEST_CASE("Gradient evaluation on chain models", "[gradient]") {
  TunableModelTest model;

  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  auto C = make_variable(2, "C");
  const float alfa = 1.f;
  model.addTunableFactor(make_corr_expfactor_ptr(A, B, alfa));
  const float beta = 0.5f;
  model.addTunableFactor(make_corr_expfactor_ptr(B, C, beta));
  {
    const std::vector<float> reference_w = {alfa, beta};

    auto modified_w =
        GENERATE(std::vector<float>{1.f, 1.f}, std::vector<float>{0.5f, 0.5f},
                 std::vector<float>{2.f, 2.f});

    CHECK(model.checkGradient(std::vector<float>{reference_w}, modified_w));
  }

  auto D = make_variable(2, "D");
  const float gamma = 2.f;
  model.addTunableFactor(make_corr_expfactor_ptr(C, D, gamma));
  {
    const std::vector<float> reference_w = {alfa, beta, gamma};

    auto modified_w = GENERATE(std::vector<float>{1.f, 1.f, 1.f},
                               std::vector<float>{0.5f, 0.5f, 0.5f},
                               std::vector<float>{2.f, 2.f, 2.f});

    CHECK(model.checkGradient(std::vector<float>{reference_w}, modified_w));
  }
}

} // namespace EFG::test
