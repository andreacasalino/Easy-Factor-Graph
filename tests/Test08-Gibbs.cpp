#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/io/xml/Importer.h>
#include <EasyFactorGraph/model/Graph.h>

#include "Utils.h"

#include <algorithm>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;
using namespace EFG::strct;
using namespace EFG::io;
using namespace EFG::test;

namespace {
bool are_samples_valid(const std::vector<Combination> &samples,
                       const VariablesSet &group) {
  if (samples.empty()) {
    return false;
  }
  auto samples_it = std::find_if(samples.begin(), samples.end(),
                                 [&group](const Combination &comb) {
                                   return comb.data().size() != group.size();
                                 });
  return samples_it == samples.end();
}

// frequency of var = 1 is returned
float getFrequency1(const std::vector<Combination> &samples,
                    const VariablesSet &group, const VariablePtr &var) {
  std::size_t varPos = std::distance(group.begin(), group.find(var));
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

TEST_CASE("binary factor gibbs sampling", "[gibbs_sampling]") {
  float w = 1.f;
  FactorExponential factor(
      Factor(Group{make_variable(2, "A"), make_variable(2, "B")},
             USE_SIMPLE_CORRELATION_TAG),
      w);

  Graph model;
  model.copyConstFactor(factor);

  SECTION("specific variable frequency") {
    model.setEvidence(model.findVariable("A"), 1);
    auto samples = model.getHiddenSetSamples(
        GibbsSampler::SamplesGenerationContext{500, 50, 0});
    REQUIRE(are_samples_valid(samples, model.getHiddenVariables()));
    CHECK(check_second_prob(1.f, expf(w),
                            getFrequency1(samples, model.getHiddenVariables(),
                                          model.findVariable("B"))));
  }

  SECTION("combinations involving all variables") {
    auto samples = model.getHiddenSetSamples(
        GibbsSampler::SamplesGenerationContext{500, 50, 0});
    model.removeAllEvidences();
    const float exp_w = expf(w);
    const float Z = 2.f * (1.f + exp_w);

    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0}),
                       exp_w / Z, 0.02f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 1}),
                       1.f / Z, 0.02f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 0}),
                       1.f / Z, 0.02f));
    CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1}),
                       exp_w / Z, 0.02f));
  }
}

TEST_CASE("simple graph gibbs sampling", "[gibbs_sampling]") {
  Graph model;

  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  auto C = make_variable(2, "C");

  float alfa = 1.2f;
  FactorExponential factor_AB(Factor(Group{A, B}, USE_SIMPLE_CORRELATION_TAG),
                              alfa);
  model.copyConstFactor(factor_AB);

  float beta = 1.5f;
  FactorExponential factor_BC(Factor(Group{B, C}, USE_SIMPLE_CORRELATION_TAG),
                              beta);
  model.copyConstFactor(factor_BC);

  auto samples = model.getHiddenSetSamples(
      GibbsSampler::SamplesGenerationContext{500, 50, 0});

  const float exp_alfa = expf(alfa);
  const float exp_beta = expf(beta);
  const float Z = 2.f * (1.f + exp_alfa + exp_beta + exp_alfa * exp_beta);

  CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0, 0}),
                     exp_alfa * exp_beta / Z, 0.02f));
  CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0, 0, 1}),
                     exp_alfa / Z, 0.02f));
  // CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0,
  // 1, 0}),
  //                    1.f / Z, 0.02f));
  // CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{0,
  // 1, 1}),
  //                    exp_beta / Z, 0.02f));

  // CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1,
  // 0, 0}),
  //                    exp_beta / Z, 0.02f));
  // CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1,
  // 0, 1}),
  //                    1.f / Z, 0.02f));
  CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1, 0}),
                     exp_alfa / Z, 0.02f));
  CHECK(almost_equal(getFrequency(samples, std::vector<std::size_t>{1, 1, 1}),
                     exp_alfa * exp_beta / Z, 0.02f));
}

TEST_CASE("polyTree gibbs sampling", "[gibbs_sampling]") {
  float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);
  Graph model;
  xml::Importer::importFromFile(model, make_graph_path("graph_1.xml"));

  auto threads = GENERATE(1, 2, 3);

  // E=1
  model.setEvidence(model.findVariable("E"), 1);
  auto samples = model.getHiddenSetSamples(
      GibbsSampler::SamplesGenerationContext{500, 20, 0}, threads);
  REQUIRE(are_samples_valid(samples, model.getHiddenVariables()));

  CHECK(check_second_prob((a * (g + e) + (1 + g * e)),
                          ((g + e) + a * (1 + g * e)),
                          getFrequency1(samples, model.getHiddenVariables(),
                                        model.findVariable("A"))));
  CHECK(check_second_prob((g + e), (1 + g * e),
                          getFrequency1(samples, model.getHiddenVariables(),
                                        model.findVariable("B"))));
  CHECK(check_second_prob((b * (g + e) + (1 + g * e)),
                          ((g + e) + b * (1 + g * e)),
                          getFrequency1(samples, model.getHiddenVariables(),
                                        model.findVariable("C"))));
  CHECK(check_second_prob(1.f, e,
                          getFrequency1(samples, model.getHiddenVariables(),
                                        model.findVariable("D"))));
}

TEST_CASE("loopy model gibbs sampling", "[gibbs_sampling]") {
  float M = expf(1.f);
  float M_alfa = powf(M, 3) + M + 2.f * powf(M, 2);
  float M_beta = powf(M, 4) + 2.f * M + powf(M, 2);
  Graph model;
  xml::Importer::importFromFile(model, make_graph_path("graph_3.xml"));

  auto threads = GENERATE(1, 2, 3);

  // E=1
  model.setEvidence(model.findVariable("E"), 1);
  auto samples = model.getHiddenSetSamples(
      GibbsSampler::SamplesGenerationContext{500, 20, 0}, threads);
  REQUIRE(are_samples_valid(samples, model.getHiddenVariables()));
  CHECK(check_second_prob(3.f * M + powf(M, 3), powf(M, 4) + 3.f * powf(M, 2),
                          getFrequency1(samples, model.getHiddenVariables(),
                                        model.findVariable("D")),
                          0.06));
  CHECK(check_second_prob(M_alfa, M_beta,
                          getFrequency1(samples, model.getHiddenVariables(),
                                        model.findVariable("C")),
                          0.06));
  CHECK(check_second_prob(M_alfa, M_beta,
                          getFrequency1(samples, model.getHiddenVariables(),
                                        model.findVariable("B")),
                          0.06));
  CHECK(check_second_prob(M * M_alfa + M_beta, M_alfa + M * M_beta,
                          getFrequency1(samples, model.getHiddenVariables(),
                                        model.findVariable("A")),
                          0.06));
}
