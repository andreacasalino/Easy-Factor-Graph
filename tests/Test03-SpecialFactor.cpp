#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/structure/SpecialFactors.h>

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

TEST_CASE("Evidence", "[factor][special]") {}

TEST_CASE("Indicator", "[factor][special]") {
  auto var = make_variable(3, "A");
  Indicator factor(var, 1);

  CombinationRawValuesMap expected_map;
  expected_map.emplace(std::vector<std::size_t>{0}, 0);
  expected_map.emplace(std::vector<std::size_t>{1}, 1.f);
  expected_map.emplace(std::vector<std::size_t>{2}, 0);
  CHECK(factor.getCombinationsMap() == expected_map);
}

TEST_CASE("Message", "[factor][special]") {
  SECTION("SUM") {}

  SECTION("MAP") {}
}
