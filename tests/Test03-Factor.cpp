#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/factor/Factor.h>

#include <Utils.h>

#include <fstream>
#include <math.h>

namespace EFG::test {
TEST_CASE("correlating factors", "[factor]") {
  std::vector<std::pair<categoric::Combination<3>, float>> expected{
      {{0, 0, 0}, 1.f}, {{0, 0, 1}, 0},   {{0, 0, 2}, 0},
      {{0, 1, 0}, 0},   {{0, 1, 1}, 0},   {{0, 1, 2}, 0},
      {{0, 2, 0}, 0},   {{0, 2, 1}, 0},   {{0, 2, 2}, 0},

      {{1, 0, 0}, 0},   {{1, 0, 1}, 0},   {{1, 0, 2}, 0},
      {{1, 1, 0}, 0},   {{1, 1, 1}, 1.f}, {{1, 1, 2}, 0},
      {{1, 2, 0}, 0},   {{1, 2, 1}, 0},   {{1, 2, 2}, 0},

      {{2, 0, 0}, 0},   {{2, 0, 1}, 0},   {{2, 0, 2}, 0},
      {{2, 1, 0}, 0},   {{2, 1, 1}, 0},   {{2, 1, 2}, 0},
      {{2, 2, 0}, 0},   {{2, 2, 1}, 0},   {{2, 2, 2}, 1.f},
  };

  SECTION("correlating factor") {
    auto factor = factor::make_simply_correlated<3>(3);

    std::vector<std::pair<categoric::Combination<3>, float>> values;
    factor.forEachCombination<false>([&values](const auto &comb, float val) {
      values.emplace_back(std::make_pair(comb, val));
    });

    CHECK(values == expected);
  }

  SECTION("anti correlating factor") {
    auto factor = factor::make_simply_anti_correlated<3>(3);
    for (auto &val : expected) {
      if (val.second == 0) {
        val.second = 1.f;
      } else {
        val.second = 0;
      }
    }

    std::vector<std::pair<categoric::Combination<3>, float>> values;
    factor.forEachCombination<false>([&values](const auto &comb, float val) {
      values.emplace_back(std::make_pair(comb, val));
    });

    CHECK(values == expected);
  }
}

TEST_CASE("probabilities computation", "[factor]") {
  std::vector<float> prob;

  SECTION("factor") {
    auto factor = factor::BinaryFactor::from_compact_domain(
        categoric::make_same<2, categoric::VarStateSize>(2),
        std::vector<float>{1.f, 0, 0, 1.f});

    getProbabilities(factor, prob);
    CHECK(test::almost_equal_it(prob, std::vector<float>{0.5f, 0, 0, 0.5f},
                                0.001f));
  }

  SECTION("exponential wrap probabilities") {
    const float w = 1.5f;

    auto factor = factor::BinaryFactorExponential::from_compact_domain(
        categoric::make_same<2, categoric::VarStateSize>(2),
        std::vector<float>{1.f, 0, 0, 1.f});
    factor.trsfm.setWeight(w);

    getProbabilities(factor, prob);
    const float big = expf(w);
    const float small = 1.f;
    const float cumul = 2.f * (small + big);
    CHECK(test::almost_equal_it(prob,
                                std::vector<float>{big / cumul, small / cumul,
                                                   small / cumul, big / cumul},
                                0.001f));
  }
}
} // namespace EFG::test
