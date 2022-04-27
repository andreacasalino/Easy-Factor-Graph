#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include "Utils.h"

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

namespace {
bool almost_equal(const CombinationRawValuesMap &a,
                  const CombinationRawValuesMap &b) {
  if (a.size() != b.size()) {
    return false;
  }
  auto b_it = b.begin();
  for (auto a_it = a.begin(); a_it != a.end(); ++a_it, ++b_it) {
    if (a_it->first != b_it->first) {
      return false;
    }
    if (!test::almost_equal(a_it->second, b_it->second, 0.01f)) {
      return false;
    }
  }
  return true;
}
} // namespace

TEST_CASE("Unary factor", "[factor-special]") {
  SECTION("ones from single variable") {
    auto var = make_variable(3, "A");
    UnaryFactor factor(var);
    REQUIRE(factor.getGroup().getVariables() == VariablesSoup{var});
    const auto &map = factor.getCombinationsMap();
    REQUIRE(map.size() == var->size());
    for (const auto &[comb, val] : factor.getCombinationsMap()) {
      CHECK(val == 1.f);
    }
  }

  SECTION("distributions merge") {
    auto var = make_variable(3, "A");

    Factor distr_1(Group{var});
    distr_1.setAllImagesRaw(1.f);
    distr_1.setImageRaw(std::vector<std::size_t>{0}, 0.1f);

    Factor distr_2(Group{var});
    distr_2.setAllImagesRaw(1.f);
    distr_2.setImageRaw(std::vector<std::size_t>{1}, 0.2f);

    Factor distr_3(Group{var});
    distr_3.setAllImagesRaw(1.f);
    distr_3.setImageRaw(std::vector<std::size_t>{2}, 0.3f);

    UnaryFactor factor({&distr_1, &distr_2, &distr_3});

    REQUIRE(factor.getGroup().getVariables() == VariablesSoup{var});

    CombinationRawValuesMap expected_map;
    expected_map.emplace(std::vector<std::size_t>{0}, 0.1f / 0.3f);
    expected_map.emplace(std::vector<std::size_t>{1}, 0.2f / 0.3f);
    expected_map.emplace(std::vector<std::size_t>{2}, 0.3f / 0.3f);
    CHECK(almost_equal(factor.getCombinationsMap(), expected_map));
  }
}

TEST_CASE("Indicator", "[factor-special]") {
  auto var = make_variable(3, "A");
  Indicator factor(var, 1);

  CombinationRawValuesMap expected_map;
  expected_map.emplace(std::vector<std::size_t>{0}, 0);
  expected_map.emplace(std::vector<std::size_t>{1}, 1.f);
  expected_map.emplace(std::vector<std::size_t>{2}, 0);
  CHECK(factor.getCombinationsMap() == expected_map);
}

namespace {
FactorExponential make_exp_test_factor(const float w) {
  Group group = {make_variable(2, "A"), make_variable(2, "B")};
  return FactorExponential{Factor{group, USE_SIMPLE_CORRELATION_TAG}, w};
}
} // namespace

TEST_CASE("Evidence", "[factor-special]") {
  const float w = 1.3f;
  auto factor = make_exp_test_factor(w);

  Evidence evidence(factor, factor.getGroup().getVariables()[0], 1);

  CHECK(evidence.getGroup().getVariables().front() ==
        factor.getGroup().getVariables()[1]);
  CombinationRawValuesMap expected_map;
  expected_map.emplace(std::vector<std::size_t>{0}, 1.f);
  expected_map.emplace(std::vector<std::size_t>{1}, exp(w));
  CHECK(almost_equal(evidence.getCombinationsMap(), expected_map));
}

TEST_CASE("Message", "[factor-special]") {
  const float w = 1.3f;
  const float g = 0.6f;

  auto factor_AB = make_exp_test_factor(w);
  auto A = factor_AB.getGroup().getVariables().front();
  auto B = factor_AB.getGroup().getVariables().back();

  Factor shape_A(Group{A});
  shape_A.setImageRaw(std::vector<std::size_t>{0}, 0.5f);
  shape_A.setImageRaw(std::vector<std::size_t>{1}, 1.f);
  FactorExponential factor_A(shape_A, g);

  UnaryFactor factor_A_normalized = UnaryFactor{{&factor_A}};
  const auto &factor_A_normalized_map =
      factor_A_normalized.getCombinationsMap();

  SECTION("SUM") {
    MessageSUM message(factor_A_normalized, factor_AB);

    const auto &map = message.getCombinationsMap();
    REQUIRE(message.getVariable().get() == B.get());
    CombinationRawValuesMap expected_map;
    expected_map.emplace(std::vector<std::size_t>{0},
                         exp(w) * factor_A_normalized_map.begin()->second +
                             factor_A_normalized_map.rbegin()->second);
    expected_map.emplace(std::vector<std::size_t>{1},
                         factor_A_normalized_map.begin()->second +
                             exp(w) * factor_A_normalized_map.rbegin()->second);
    const auto &temp = message.getCombinationsMap();
    CHECK(almost_equal(message.getCombinationsMap(), expected_map));
  }

  SECTION("MAP") {
    MessageMAP message(factor_A_normalized, factor_AB);

    const auto &map = message.getCombinationsMap();
    REQUIRE(message.getVariable().get() == B.get());
    CombinationRawValuesMap expected_map;
    expected_map.emplace(
        std::vector<std::size_t>{0},
        std::max(exp(w) * factor_A_normalized_map.begin()->second,
                 factor_A_normalized_map.rbegin()->second));
    expected_map.emplace(
        std::vector<std::size_t>{1},
        std::max(factor_A_normalized_map.begin()->second,
                 exp(w) * factor_A_normalized_map.rbegin()->second));
    CHECK(almost_equal(message.getCombinationsMap(), expected_map));
  }
}
