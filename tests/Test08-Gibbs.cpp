#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/model/Graph.h>

#include "ModelLibrary.h"
#include "Utils.h"

#include <algorithm>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;
using namespace EFG::strct;
using namespace EFG::test;
using namespace EFG::test::library;

namespace {
bool are_samples_valid(const std::vector<Combination> &samples,
                       const VariablesSoup &group) {
  if (samples.empty()) {
    return false;
  }
  auto samples_it = std::find_if(
      samples.begin(), samples.end(), [&group](const Combination &comb) {
        if (comb.data().size() != group.size()) {
          return true;
        }
        for (std::size_t k = 0; k < comb.size(); ++k) {
          if (group[k]->size() <= comb.data()[k]) {
            return true;
          }
        }
        return false;
      });
  return samples_it == samples.end();
}

// frequency of var = 1 is returned
float getFrequency1(const std::vector<Combination> &samples,
                    const VariablesSoup &group, const VariablePtr &var) {
  auto var_it = std::find(group.begin(), group.end(), var);
  std::size_t varPos = std::distance(group.begin(), var_it);
  std::size_t instances = 0;
  for (const auto &sample : samples) {
    if (1 == sample.data()[varPos]) {
      ++instances;
    }
  }
  return static_cast<float>(instances) / static_cast<float>(samples.size());
};

float getFrequency(const std::vector<Combination> &samples,
                   const std::vector<std::size_t> &comb_to_search) {
  std::size_t result = 0;
  for (const auto &sample : samples) {
    if (sample.data() == comb_to_search) {
      ++result;
    }
  }
  float result2 =
      static_cast<float>(result) / static_cast<float>(samples.size());
  return result2;
}

bool check_second_prob(const float expected_value_0,
                       const float expected_value_1, const float freq_1,
                       const float threshold = 0.05f) {
  return almost_equal(
      (*ProbDistribution{{expected_value_0, expected_value_1}})[1], freq_1,
      threshold);
}
} // namespace

TEST_CASE("binary factor gibbs sampling", "[gibbs_sampling][!mayfail]") {
  Graph model;

  auto w = GENERATE(0.5f, 1.f, 2.f);
  model.addConstFactor(
      make_corr_expfactor2(make_variable(2, "A"), make_variable(2, "B"), w));

  SECTION("combinations involving all variables") {
    auto samples =
        model.makeSamples(GibbsSampler::SamplesGenerationContext{500, 50, 0});
    model.removeAllEvidences();
    const float exp_w = expf(w);
    const float Z = 2.f * (1.f + exp_w);

    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0}),
                       exp_w / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 1}),
                       1.f / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 0}),
                       1.f / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1}),
                       exp_w / Z, 0.05f));
  }

  SECTION("specific variable frequency") {
    model.setEvidence(model.findVariable("A"), 1);
    auto samples =
        model.makeSamples(GibbsSampler::SamplesGenerationContext{500, 50, 0});
    REQUIRE(are_samples_valid(samples, model.getAllVariables()));
    CHECK(check_second_prob(1.f, expf(w),
                            getFrequency1(samples, model.getAllVariables(),
                                          model.findVariable("B"))));
  }
}

namespace {
float getFrequency(const std::vector<Combination> &samples,
                   const std::vector<std::size_t> &comb_to_search,
                   const std::vector<std::size_t> &pos_to_search) {
  std::size_t result = 0;
  for (const auto &sample : samples) {
    std::vector<std::size_t> sub_part;
    sub_part.reserve(pos_to_search.size());
    for (const auto &pos : pos_to_search) {
      sub_part.push_back(sample.data()[pos]);
    }
    if (sub_part == comb_to_search) {
      ++result;
    }
  }
  float result2 =
      static_cast<float>(result) / static_cast<float>(samples.size());
  return result2;
}
} // namespace

TEST_CASE("2 binary factors gibbs sampling", "[gibbs_sampling][!mayfail]") {
  Graph model;

  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  auto C = make_variable(2, "C");

  float alfa = 0.3f;
  model.addConstFactor(make_corr_expfactor2(A, B, alfa));

  float beta = 1.3f;
  model.addConstFactor(make_corr_expfactor2(B, C, beta));

  auto samples =
      model.makeSamples(GibbsSampler::SamplesGenerationContext{1000, 50, 0});

  const float exp_alfa = expf(alfa);
  const float exp_beta = expf(beta);

  {
    const float Z = 2.f * (1.f + exp_alfa);

    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0},
                                    std::vector<std::size_t>{0, 1}),
                       exp_alfa / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1},
                                    std::vector<std::size_t>{0, 1}),
                       exp_alfa / Z, 0.05f));
  }

  {
    const float Z = 2.f * (1.f + exp_beta);

    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0},
                                    std::vector<std::size_t>{1, 2}),
                       exp_beta / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1},
                                    std::vector<std::size_t>{1, 2}),
                       exp_beta / Z, 0.05f));
  }

  {
    const float Z = 2.f * (1.f + exp_alfa + exp_beta + exp_alfa * exp_beta);

    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0, 0}),
                       exp_alfa * exp_beta / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0, 1}),
                       exp_alfa / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 1, 0}),
                       1.f / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 1, 1}),
                       exp_beta / Z, 0.05f));

    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 0, 0}),
                       exp_beta / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 0, 1}),
                       1.f / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1, 0}),
                       exp_alfa / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1, 1}),
                       exp_alfa * exp_beta / Z, 0.05f));
  }
}

TEST_CASE("polyTree gibbs sampling", "[gibbs_sampling][!mayfail]") {
  RandomField model(SIMPLE_TREE);

  const float a = expf(SimpleTree::alfa);
  const float b = expf(SimpleTree::beta);
  const float g = expf(SimpleTree::gamma);
  const float e = expf(SimpleTree::eps);

  auto threads = GENERATE(1, 2);
  float toll = (threads > 1) ? 0.1f : 0.06f;

  // E=1
  model.setEvidence(model.findVariable("E"), 1);
  auto samples = model.makeSamples(
      GibbsSampler::SamplesGenerationContext{1500, 50, 0}, threads);
  REQUIRE(are_samples_valid(samples, model.getAllVariables()));

  CHECK(check_second_prob(
      (a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e)),
      getFrequency1(samples, model.getAllVariables(), model.findVariable("A")),
      toll));
  CHECK(check_second_prob(
      (g + e), (1 + g * e),
      getFrequency1(samples, model.getAllVariables(), model.findVariable("B")),
      toll));
  CHECK(check_second_prob(
      (b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e)),
      getFrequency1(samples, model.getAllVariables(), model.findVariable("C")),
      toll));
  CHECK(check_second_prob(
      1.f, e,
      getFrequency1(samples, model.getAllVariables(), model.findVariable("D")),
      toll));
}

TEST_CASE("loopy model gibbs sampling", "[gibbs_sampling][!mayfail]") {
  RandomField model(SIMPLE_LOOPY);

  float M = expf(SimpleLoopy::w);
  float M_alfa = powf(M, 3) + M + 2.f * powf(M, 2);
  float M_beta = powf(M, 4) + 2.f * M + powf(M, 2);

  auto threads = GENERATE(1, 2);
  float toll = (threads > 1) ? 0.1f : 0.06f;

  // E=1
  model.setEvidence(model.findVariable("E"), 1);
  auto samples = model.makeSamples(
      GibbsSampler::SamplesGenerationContext{1500, 50, 0}, threads);
  REQUIRE(are_samples_valid(samples, model.getAllVariables()));
  CHECK(check_second_prob(
      3.f * M + powf(M, 3), powf(M, 4) + 3.f * powf(M, 2),
      getFrequency1(samples, model.getAllVariables(), model.findVariable("D")),
      toll));
  CHECK(check_second_prob(
      M_alfa, M_beta,
      getFrequency1(samples, model.getAllVariables(), model.findVariable("C")),
      toll));
  CHECK(check_second_prob(
      M_alfa, M_beta,
      getFrequency1(samples, model.getAllVariables(), model.findVariable("B")),
      toll));
  CHECK(check_second_prob(
      M * M_alfa + M_beta, M_alfa + M * M_beta,
      getFrequency1(samples, model.getAllVariables(), model.findVariable("A")),
      toll));
}
