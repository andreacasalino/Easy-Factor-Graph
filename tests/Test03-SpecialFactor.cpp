#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include <math.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

TEST_CASE("Unary factor", "[factor][special]") {
  SECTION("ones from single variable") {
    auto var = make_variable(3, "A");
    UnaryFactor factor(var);
    REQUIRE(factor.getVariables().getVariables() == VariablesSoup{var});
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

    REQUIRE(factor.getVariables().getVariables() == VariablesSoup{var});
    const auto &map = factor.getCombinationsMap();
    REQUIRE(map.size() == var->size());

    CombinationRawValuesMap expected_map;
    expected_map.emplace(std::vector<std::size_t>{0}, 0.1f);
    expected_map.emplace(std::vector<std::size_t>{1}, 0.2f);
    expected_map.emplace(std::vector<std::size_t>{2}, 0.3f);
    CHECK(factor.getCombinationsMap() == expected_map);
  }
}

TEST_CASE("Indicator", "[factor][special]") {
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

TEST_CASE("Evidence", "[factor][special]") {
  const float w = 1.3f;
  auto factor = make_exp_test_factor(w);

  Evidence evidence(factor, factor.getVariables().getVariables()[0], 1);

  const auto &map = evidence.getCombinationsMap();
  REQUIRE(map.size() == 2);
  CombinationRawValuesMap expected_map;
  expected_map.emplace(std::vector<std::size_t>{0}, 1.f);
  expected_map.emplace(std::vector<std::size_t>{1}, 1.f + exp(w));
  CHECK(factor.getCombinationsMap() == expected_map);
}

TEST_CASE("Message", "[factor][special]") {
  const float w = 1.3f;
  const float g = 0.6f;

  auto factor_AB = make_exp_test_factor(w);
  auto A = factor_AB.getVariables().getVariables().front();
  auto B = factor_AB.getVariables().getVariables().back();

  Factor shape_A(Group{A});
  shape_A.setImageRaw(std::vector<std::size_t>{0}, 0.5f);
  shape_A.setImageRaw(std::vector<std::size_t>{1}, 1.f);
  FactorExponential factor_A(shape_A, g);

  SECTION("SUM") {
    MessageSUM message(UnaryFactor{{&factor_A}}, factor_AB);

    const auto &map = message.getCombinationsMap();
    REQUIRE(message.getVariable().get() == B.get());
    REQUIRE(map.size() == 2);
    CombinationRawValuesMap expected_map;
    expected_map.emplace(std::vector<std::size_t>{0},
                         exp(w) * exp(0.5f * g) + exp(g));
    expected_map.emplace(std::vector<std::size_t>{1},
                         exp(0.5f * g) + exp(w) * exp(g));
    CHECK(message.getCombinationsMap() == expected_map);
  }

  SECTION("MAP") {
    MessageMAP message(UnaryFactor{{&factor_A}}, factor_AB);

    const auto &map = message.getCombinationsMap();
    REQUIRE(message.getVariable().get() == B.get());
    REQUIRE(map.size() == 2);
    CombinationRawValuesMap expected_map;
    expected_map.emplace(std::vector<std::size_t>{0},
                         std::max(exp(w) * exp(0.5f * g), exp(g)));
    expected_map.emplace(std::vector<std::size_t>{1},
                         std::max(exp(0.5f * g), exp(w) * exp(g)));
    CHECK(message.getCombinationsMap() == expected_map);
  }
}
