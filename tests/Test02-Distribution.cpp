#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <math.h>

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/Factor.h>
#include <EasyFactorGraph/distribution/FactorExponential.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

namespace {
Factor make_factor(const VariablesSoup &vars,
                   const CombinationRawValuesMap &map) {
  Factor result(Group{vars});
  for (const auto &[comb, val] : map) {
    result.setImageRaw(comb, val);
  }
  return result;
}

} // namespace

TEST_CASE("operations on distribution", "[distribution]") {
  const VariablePtr A = make_variable(3, "A");

  SECTION("unary factor") {
    auto factor = make_factor({A}, {{std::vector<std::size_t>{0}, 1.f},
                                    {std::vector<std::size_t>{1}, 2.f}});

    CHECK(factor.evaluate(std::vector<std::size_t>{0}) == 1.f);
    CHECK(factor.evaluate(std::vector<std::size_t>{1}) == 2.f);
    CHECK(factor.evaluate(std::vector<std::size_t>{2}) == 0);
  }

  const VariablePtr B = make_variable(4, "B");

  SECTION("binary factor") {
    auto factor = make_factor({A, B}, {{std::vector<std::size_t>{0, 1}, 1.f},
                                       {std::vector<std::size_t>{1, 1}, 2.f},
                                       {std::vector<std::size_t>{1, 3}, 0.5f},
                                       {std::vector<std::size_t>{1, 2}, 0.7f}});

    CHECK(factor.evaluate(std::vector<std::size_t>{0, 1}) == 1.f);
    CHECK(factor.evaluate(std::vector<std::size_t>{1, 1}) == 2.f);
    CHECK(factor.evaluate(std::vector<std::size_t>{1, 3}) == 0.5f);
    CHECK(factor.evaluate(std::vector<std::size_t>{1, 2}) == 0.7f);

    CHECK(factor.evaluate(std::vector<std::size_t>{0, 2}) == 0);
    CHECK(factor.evaluate(std::vector<std::size_t>{0, 3}) == 0);
  }

  const VariablePtr C = make_variable(2, "C");
  SECTION("ternary factor") {
    auto factor =
        make_factor({A, B, C}, {{std::vector<std::size_t>{0, 1, 0}, 1.f},
                                {std::vector<std::size_t>{1, 1, 1}, 2.f},
                                {std::vector<std::size_t>{1, 3, 0}, 0.5f},
                                {std::vector<std::size_t>{1, 2, 1}, 0.7f}});

    CHECK(factor.evaluate(std::vector<std::size_t>{0, 1, 0}) == 1.f);
    CHECK(factor.evaluate(std::vector<std::size_t>{1, 1, 1}) == 2.f);
    CHECK(factor.evaluate(std::vector<std::size_t>{1, 3, 0}) == 0.5f);
    CHECK(factor.evaluate(std::vector<std::size_t>{1, 2, 1}) == 0.7f);

    CHECK(factor.evaluate(std::vector<std::size_t>{0, 2, 0}) == 0);
    CHECK(factor.evaluate(std::vector<std::size_t>{0, 2, 1}) == 0);
    CHECK(factor.evaluate(std::vector<std::size_t>{0, 3, 0}) == 0);
    CHECK(factor.evaluate(std::vector<std::size_t>{0, 3, 1}) == 0);

    SECTION("variables substitution") {
      const VariablesSoup new_vars =
          VariablesSoup{make_variable(3, "A2"), make_variable(4, "B2"),
                        make_variable(2, "C2")};
      factor.replaceVariables(new_vars);
      CHECK(factor.getVariables().getVariables()[0]->name() == "A2");
      CHECK(factor.getVariables().getVariables()[1]->name() == "B2");
      CHECK(factor.getVariables().getVariables()[2]->name() == "C2");

      const VariablesSoup new_vars_bad =
          VariablesSoup{make_variable(5, "A2"), make_variable(2, "B2"),
                        make_variable(2, "C2")};
      CHECK_THROWS_AS(factor.replaceVariables(new_vars_bad), Error);
    }

    SECTION("set onse") {
      factor.setAllImagesRaw(1.f);
      const auto &factor_map = factor.getCombinationsMap();
      CHECK(factor_map.size() == factor.getVariables().size());
      GroupRange range(factor.getVariables());
      for_each_combination(range, [&factor_map](const Combination &comb) {
        auto factor_map_it = factor_map.find(comb);
        REQUIRE(factor_map_it != factor_map.end());
        CHECK(factor_map_it->second == 1.f);
      });
    }

    SECTION("exponential wrap evaluation") {
      const float w = 1.5f;
      FactorExponential factor_exp(factor, w);

      CHECK(factor_exp.evaluate(std::vector<std::size_t>{0, 1, 0}) ==
            expf(w * 1.f));
      CHECK(factor_exp.evaluate(std::vector<std::size_t>{1, 1, 1}) ==
            expf(w * 2.f));
    }
  }
}

namespace {
bool are_similars(const std::vector<float> &a, const std::vector<float> &b) {
  if (a.size() != b.size()) {
    return false;
  }
  for (std::size_t k = 0; k < a.size(); ++k) {
    if (abs(a[k] - b[k]) > 1e-2) {
      return false;
    }
  }
  return true;
}
} // namespace

TEST_CASE("probabilties computation", "[distribution]") {
  auto factor =
      make_factor(VariablesSoup{make_variable(2, "A"), make_variable(2, "B")},
                  {{std::vector<std::size_t>{0, 0}, 1.f},
                   {std::vector<std::size_t>{0, 1}, 0.f},
                   {std::vector<std::size_t>{1, 0}, 0.f},
                   {std::vector<std::size_t>{1, 1}, 1.f}});

  SECTION("probabilities") {
    CHECK(are_similars(factor.getProbabilities(), {0.5f, 0, 0, 0.5f}));
  }

  SECTION("exponential wrap probabilities") {
    const float w = 1.5f;
    FactorExponential factor_exp(factor, w);

    const float big = expf(w);
    const float small = 1.f;
    const float cumul = 2.f * (small + big);
    CHECK(
        are_similars(factor_exp.getProbabilities(),
                     {big / cumul, small / cumul, small / cumul, big / cumul}));
  }
}

#include <EasyFactorGraph/distribution/CombinationFinder.h>

namespace {
categoric::Combination make_combination(const std::size_t size,
                                        const std::size_t val) {
  std::vector<std::size_t> result;
  result.reserve(size);
  for (std::size_t k = 0; k < size; ++k) {
    result.push_back(val);
  }
  return result;
}
} // namespace

TEST_CASE("bigger combination finder", "[distribution]") {
  auto group_size = GENERATE(1, 2, 3);

  VariablesSoup group;
  for (int k = 0; k < group_size; ++k) {
    group.push_back(make_variable(4, "V" + std::to_string(k)));
  }
  Factor factor(Group{group});
  for (std::size_t k = 0; k < 2; ++k) {
    factor.setImageRaw(make_combination(group_size, k), 1.f);
  }

  VariablesSoup bigger_group;
  for (int k = 0; k < group_size; ++k) {
    bigger_group.push_back(group[k]);
    bigger_group.push_back(make_variable(4, "V" + std::to_string(k) + "_bis"));
  }
  auto factor_finder = factor.makeFinder(bigger_group);

  {
    // 0, ... ,0
    auto found = factor_finder.find(make_combination(bigger_group.size(), 0));
    CHECK(found.map_iterator != factor.getCombinationsMap().end());
    CHECK(found.value == 1.f);
  }

  {
    // 1, ... ,1
    auto found = factor_finder.find(make_combination(bigger_group.size(), 1));
    CHECK(found.map_iterator != factor.getCombinationsMap().end());
    CHECK(found.value == 1.f);
  }

  {
    // 2, ... ,2
    auto found = factor_finder.find(make_combination(bigger_group.size(), 2));
    CHECK(found.map_iterator == factor.getCombinationsMap().end());
    CHECK(found.value == 0);
  }

  {
    // 3, ... ,3
    auto found = factor_finder.find(make_combination(bigger_group.size(), 3));
    CHECK(found.map_iterator == factor.getCombinationsMap().end());
    CHECK(found.value == 0);
  }
}
