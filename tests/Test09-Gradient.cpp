#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Utils.h"
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <EasyFactorGraph/model/RandomField.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>
#include <EasyFactorGraph/trainable/tuners/BaseTuner.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::train;
using namespace EFG::strct;
using namespace EFG::model;
using namespace EFG::test;

namespace {
class TunableModelTest : public RandomField {
public:
  TunableModelTest() = default;

  TrainSet makeGoodTrainset(const std::size_t samples) const {
    auto combs_and_prob = compute_combinations_and_probs(*this);
    std::vector<categoric::Combination> sampled;
    sampled.reserve(samples);
    UniformSampler sampler;
    sampler.resetSeed(0);
    while (sampled.size() != samples) {
      auto pos = sampler.sampleFromDiscrete(combs_and_prob.probs);
      sampled.push_back(combs_and_prob.combinations[pos]);
    }
    return TrainSet{sampled};
  }

  bool checkGradient(const std::vector<float> &for_samples_generation,
                     const std::vector<float> &for_gradient_computation) {
    setWeights(for_samples_generation);
    auto samples = makeGoodTrainset(1000);

    propagateBelief(SUM);
    auto samples_it = samples.makeIterator();
    setWeights(for_gradient_computation);
    for (std::size_t t = 0; t < tuners.size(); ++t) {
      const auto &tuner = tuners[t];

      float alfa_part = tuner->getGradientAlpha(samples_it);
      float alfa_expected = expf(for_samples_generation[t]) /
                            (1.f + expf(for_samples_generation[t]));

      float beta_part = tuner->getGradientBeta();
      float beta_expected =
          expf(tuner->getWeight()) / (1.f + expf(tuner->getWeight()));

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

TEST_CASE("Gradient evaluation on binary factor", "[train][gradient]") {
  TunableModelTest model;

  const float w = 1.5f;
  model.addTunableFactor(
      make_corr_expfactor2(make_variable(2, "A"), make_variable(2, "B"), w));

  auto grad_w = GENERATE(0.5f, 2.f);

  CHECK(model.checkGradient(std::vector<float>{w}, std::vector<float>{grad_w}));
}

TEST_CASE("Gradient evaluation on a simple chain model", "[train][gradient]") {
  TunableModelTest model;

  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  auto C = make_variable(2, "C");
  auto D = make_variable(2, "D");

  const float alfa = 1.f;
  model.addTunableFactor(make_corr_expfactor2(A, B, alfa));
  const float beta = 0.5f;
  model.addTunableFactor(make_corr_expfactor2(B, C, beta));
  const float gamma = 2.f;
  model.addTunableFactor(make_corr_expfactor2(C, D, gamma));

  const std::vector<float> reference_w = {alfa, beta, gamma};

  auto grad_w = GENERATE(std::vector<float>{1.f, 1.f, 1.f},
                         std::vector<float>{0.5f, 0.5f, 0.5f},
                         std::vector<float>{2.f, 2.f, 2.f});

  CHECK(model.checkGradient(std::vector<float>{reference_w}, grad_w));
}
