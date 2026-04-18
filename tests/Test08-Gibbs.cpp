#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/misc/Generator.h>
#include <EasyFactorGraph/model/Graph.h>

#include <ModelLibrary.h>
#include <Utils.h>

#include <algorithm>

namespace EFG::test {
using namespace misc;
using namespace categoric;
using namespace structure;
using namespace model;

namespace {
bool are_samples_valid(Samples::SamplesIter samples,
                       const std::vector<std::size_t> group_sizes) {
  bool contains_sample{false};
  while (true) {
    auto next_sample = samples.next();
    if (!next_sample.has_value()) {
      break;
    }
    contains_sample = true;
    if (next_sample->size() != group_sizes.size()) {
      return false;
    }
    for (std::size_t k = 0; k < next_sample->size(); ++k) {
      if (group_sizes[k] <= (*next_sample)[k]) {
        return true;
      }
    }
  }
  return contains_sample;
}

// frequency of var = 1 is returned
float getFrequency1(Samples::SamplesIter samples, std::size_t var_pos) {
  std::size_t instances{0}, size{0};
  for_each_generated<std::span<const VarStateSize>>(
      std::move(samples), [&](const auto &sample) {
        if (sample[var_pos] == 1) {
          ++instances;
        }
        size += 1;
      });
  return static_cast<float>(instances) / static_cast<float>(size);
};

float getFrequency(Samples::SamplesIter samples,
                   const std::vector<VarStateSize> &to_find) {
  std::size_t instances{0}, size{0};
  for_each_generated<std::span<const VarStateSize>>(
      std::move(samples), [&](const auto &sample) {
        if (are_same(sample, std::span<const VarStateSize>{to_find})) {
          ++instances;
        }
        size += 1;
      });
  return static_cast<float>(instances) / static_cast<float>(size);
}

bool check_second_prob(float expected_value_0, float expected_value_1,
                       float freq_1, float threshold = 0.05f) {
  return almost_equal(make_prob_distr({expected_value_0, expected_value_1})[1],
                      freq_1, threshold);
}
} // namespace

TEST_CASE("binary factor gibbs sampling", "[gibbs_sampling]") {
  auto w = GENERATE(0.5f, 1.f, 2.f);
  LineModel model{2, w};

  SECTION("combinations involving all variables") {
    auto samples =
        model.makeSamples(GibbsSampler::SamplesGenerationContext{500, 50, 0});
    const float exp_w = expf(w);
    const float Z = 2.f * (1.f + exp_w);

    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{0, 0}),
        exp_w / Z, 0.075f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{0, 1}),
        1.f / Z, 0.075f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{1, 0}),
        1.f / Z, 0.075f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{1, 1}),
        exp_w / Z, 0.075f));
  }

  SECTION("specific variable frequency") {
    model.setEvidences(structure::Evidence{0, 1});
    auto samples =
        model.makeSamples(GibbsSampler::SamplesGenerationContext{500, 50, 0});
    const float exp_w = expf(w);
    const float Z = 1.f + exp_w;

    REQUIRE(are_samples_valid(samples->makeIter(),
                              {model.getStructure().nodes[1].var_size}));
    CHECK(almost_equal(expf(w) / Z, getFrequency1(samples->makeIter(), 0),
                       0.075f));
  }
}

namespace {
float getFrequency(Samples::SamplesIter samples,
                   const std::vector<VarStateSize> &comb_to_search,
                   const std::vector<std::size_t> &pos_to_search) {
  std::size_t result = 0;
  std::size_t total = 0;
  for_each_generated<std::span<const VarStateSize>>(
      std::move(samples), [&](const auto &sample) {
        total += 1;
        for (std::size_t i{0}; i < pos_to_search.size(); ++i) {
          if (sample[pos_to_search[i]] != comb_to_search[comb_to_search[i]]) {
            return;
          }
        }
        result += 1;
      });
  float result2 = static_cast<float>(result) / static_cast<float>(total);
  return result2;
}

std::vector<std::size_t> getHiddenSetSizes(const Structure &structure) {
  auto rng =
      structure.nodes | std::views::filter([](const Node &node) {
        return node.evidence == Evidence::NOT_AN_EVIDENCE;
      }) |
      std::views::transform([](const Node &node) { return node.var_size; });
  return {rng.begin(), rng.end()};
}
} // namespace

TEST_CASE("Two binary factors gibbs sampling", "[gibbs_sampling]") {
  float alfa = 0.3f;
  float beta = 1.3f;

  LineModel model{2, {alfa, beta}};

  auto samples =
      model.makeSamples(GibbsSampler::SamplesGenerationContext{1000, 50, 0});

  const float exp_alfa = expf(alfa);
  const float exp_beta = expf(beta);

  SECTION("Check freq of sub-combination") {
    const float Z = 2.f * (1.f + exp_alfa);

    CHECK(almost_equal(getFrequency(samples->makeIter(),
                                    std::vector<VarStateSize>{0, 0},
                                    std::vector<std::size_t>{0, 1}),
                       exp_alfa / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples->makeIter(),
                                    std::vector<VarStateSize>{1, 1},
                                    std::vector<std::size_t>{0, 1}),
                       exp_alfa / Z, 0.05f));
  }

  SECTION("Check freq of sub-combination") {
    const float Z = 2.f * (1.f + exp_beta);

    CHECK(almost_equal(getFrequency(samples->makeIter(),
                                    std::vector<VarStateSize>{0, 0},
                                    std::vector<std::size_t>{1, 2}),
                       exp_beta / Z, 0.05f));
    CHECK(almost_equal(getFrequency(samples->makeIter(),
                                    std::vector<VarStateSize>{1, 1},
                                    std::vector<std::size_t>{1, 2}),
                       exp_beta / Z, 0.05f));
  }

  SECTION("Check freq of all distinct combinations") {
    const float Z = 2.f * (1.f + exp_alfa + exp_beta + exp_alfa * exp_beta);

    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{0, 0, 0}),
        exp_alfa * exp_beta / Z, 0.05f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{0, 0, 1}),
        exp_alfa / Z, 0.05f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{0, 1, 0}),
        1.f / Z, 0.05f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{0, 1, 1}),
        exp_beta / Z, 0.05f));

    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{1, 0, 0}),
        exp_beta / Z, 0.05f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{1, 0, 1}),
        1.f / Z, 0.05f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{1, 1, 0}),
        exp_alfa / Z, 0.05f));
    CHECK(almost_equal(
        getFrequency(samples->makeIter(), std::vector<VarStateSize>{1, 1, 1}),
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

  auto activator = activate_if_needed(model, threads);

  // E=1
  model.setEvidences(
      Evidence{model.getStructure().named_vars_table.at("E"), 1});
  auto samples =
      model.makeSamples(GibbsSampler::SamplesGenerationContext{1500, 50, 0});
  REQUIRE(are_samples_valid(samples->makeIter(),
                            getHiddenSetSizes(model.getStructure())));

  CHECK(check_second_prob(
      (a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e)),
      getFrequency1(samples->makeIter(),
                    model.getStructure().named_vars_table.at("A")),
      toll));
  CHECK(check_second_prob(
      (g + e), (1 + g * e),
      getFrequency1(samples->makeIter(),
                    model.getStructure().named_vars_table.at("B")),
      toll));
  CHECK(check_second_prob(
      (b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e)),
      getFrequency1(samples->makeIter(),
                    model.getStructure().named_vars_table.at("C")),
      toll));
  CHECK(check_second_prob(
      1.f, e,
      getFrequency1(samples->makeIter(),
                    model.getStructure().named_vars_table.at("D")),
      toll));
}

TEST_CASE("loopy model gibbs sampling", "[gibbs_sampling]") {
  SimpleLoopy model;

  float M = expf(SimpleLoopy::w);
  float M_alfa = powf(M, 3) + M + 2.f * powf(M, 2);
  float M_beta = powf(M, 4) + 2.f * M + powf(M, 2);

  auto threads = GENERATE(1, 2);
  float toll = (threads > 1) ? 0.1f : 0.06f;

  auto activator = activate_if_needed(model, threads);

  // E=1
  model.setEvidences(
      Evidence{model.getStructure().named_vars_table.at("E"), 1});
  auto samples =
      model.makeSamples(GibbsSampler::SamplesGenerationContext{1500, 50, 0});
  REQUIRE(are_samples_valid(samples->makeIter(),
                            getHiddenSetSizes(model.getStructure())));

  CHECK(check_second_prob(
      3.f * M + powf(M, 3), powf(M, 4) + 3.f * powf(M, 2),
      getFrequency1(samples->makeIter(),
                    model.getStructure().named_vars_table.at("D")),
      toll));
  CHECK(check_second_prob(
      M_alfa, M_beta,
      getFrequency1(samples->makeIter(),
                    model.getStructure().named_vars_table.at("C")),
      toll));
  CHECK(check_second_prob(
      M_alfa, M_beta,
      getFrequency1(samples->makeIter(),
                    model.getStructure().named_vars_table.at("B")),
      toll));
  CHECK(check_second_prob(
      M * M_alfa + M_beta, M_alfa + M * M_beta,
      getFrequency1(samples->makeIter(),
                    model.getStructure().named_vars_table.at("A")),
      toll));
}
} // namespace EFG::test
