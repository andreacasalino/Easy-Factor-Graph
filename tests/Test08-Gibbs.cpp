#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/model/Graph.h>

#include "ModelLibrary.h"
#include "Utils.h"

#include <algorithm>

namespace EFG::test {
using namespace categoric;
using namespace model;
using namespace strct;
using namespace library;

namespace {
bool are_samples_valid(const std::vector<std::vector<std::size_t>> &samples,
                       const VariablesSoup &group) {
  if (samples.empty()) {
    return false;
  }

  auto bad_comb = [&group](const std::vector<std::size_t> &comb) {
    if (comb.size() != group.size()) {
      return true;
    }
    for (std::size_t k = 0; k < comb.size(); ++k) {
      if (group[k]->size() <= comb[k]) {
        return true;
      }
    }
    return false;
  };

  return std::find_if(samples.begin(), samples.end(), bad_comb) ==
         samples.end();
}

// frequency of var = 1 is returned
float getFrequency1(const std::vector<std::vector<std::size_t>> &samples,
                    const VariablesSoup &group, const VariablePtr &var) {
  std::size_t varPos;
  if (auto it = std::find(group.begin(), group.end(), var); it != group.end()) {
    varPos = std::distance(group.begin(), it);
  } else {
    throw Error::make("Unable to find variable ", var->name());
  }
  std::size_t instances = 0;
  for (const auto &sample : samples) {
    if (sample[varPos] == 1) {
      ++instances;
    }
  }
  return static_cast<float>(instances) / static_cast<float>(samples.size());
};

float getFrequency(const std::vector<std::vector<std::size_t>> &samples,
                   const std::vector<std::size_t> &comb_to_search) {
  std::size_t result = 0;
  for (const auto &sample : samples) {
    if (sample == comb_to_search) {
      ++result;
    }
  }
  float result2 =
      static_cast<float>(result) / static_cast<float>(samples.size());
  return result2;
}

bool check_second_prob(float expected_value_0, float expected_value_1,
                       float freq_1, float threshold = 0.05f) {
  return almost_equal(make_prob_distr({expected_value_0, expected_value_1})[1],
                      freq_1, threshold);
}
} // namespace

TEST_CASE("binary factor gibbs sampling", "[gibbs_sampling]") {
  Graph model;

  auto w = GENERATE(0.5f, 1.f, 2.f);
  model.addConstFactor(
      make_corr_expfactor_ptr(make_variable(2, "A"), make_variable(2, "B"), w));

  SECTION("combinations involving all variables") {
    auto samples =
        model.makeSamples(GibbsSampler::SamplesGenerationContext{500, 50, 0});
    model.removeAllEvidences();
    const float exp_w = expf(w);
    const float Z = 2.f * (1.f + exp_w);

    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0}),
                       exp_w / Z, 0.075f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 1}),
                       1.f / Z, 0.075f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 0}),
                       1.f / Z, 0.075f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1}),
                       exp_w / Z, 0.075f));
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
float getFrequency(const std::vector<std::vector<std::size_t>> &samples,
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

TEST_CASE("2 binary factors gibbs sampling", "[gibbs_sampling]") {
  Graph model;

  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  auto C = make_variable(2, "C");

  float alfa = 0.3f;
  model.addConstFactor(make_corr_expfactor_ptr(A, B, alfa));

  float beta = 1.3f;
  model.addConstFactor(make_corr_expfactor_ptr(B, C, beta));

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

TEST_CASE("polyTree gibbs sampling", "[gibbs_sampling]") {
  SimpleTree model;

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

TEST_CASE("loopy model gibbs sampling", "[gibbs_sampling]") {
  SimpleLoopy model;

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

} // namespace EFG::test
