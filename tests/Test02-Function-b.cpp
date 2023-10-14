#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <math.h>

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/FactorExponential.h>

#include "Utils.h"

namespace EFG::test {
using namespace categoric;
using namespace factor;

namespace {
using CombinationsMap = std::vector<std::pair<std::vector<std::size_t>, float>>;

Factor make_factor(const VariablesSoup &vars, const CombinationsMap &map) {
  Factor result(Group{vars});
  for (const auto &[comb, img] : map) {
    result.set(comb, img);
  }
  return result;
}

} // namespace

TEST_CASE("operations on distribution", "[function]") {
  const VariablePtr A = make_variable(3, "A");

  SECTION("unary factor") {
    auto factor = make_factor({A}, {{std::vector<std::size_t>{0}, 1.f},
                                    {std::vector<std::size_t>{1}, 2.f}});

    CHECK(factor.function().findImage(std::vector<std::size_t>{0}) == 1.f);
    CHECK(factor.function().findImage(std::vector<std::size_t>{1}) == 2.f);
    CHECK(factor.function().findImage(std::vector<std::size_t>{2}) == 0);
  }

  const VariablePtr B = make_variable(4, "B");

  SECTION("binary factor") {
    auto factor = make_factor({A, B}, {{std::vector<std::size_t>{0, 1}, 1.f},
                                       {std::vector<std::size_t>{1, 1}, 2.f},
                                       {std::vector<std::size_t>{1, 3}, 0.5f},
                                       {std::vector<std::size_t>{1, 2}, 0.7f}});

    CHECK(factor.function().findImage(std::vector<std::size_t>{0, 1}) == 1.f);
    CHECK(factor.function().findImage(std::vector<std::size_t>{1, 1}) == 2.f);
    CHECK(factor.function().findImage(std::vector<std::size_t>{1, 3}) == 0.5f);
    CHECK(factor.function().findImage(std::vector<std::size_t>{1, 2}) == 0.7f);

    CHECK(factor.function().findImage(std::vector<std::size_t>{0, 2}) == 0);
    CHECK(factor.function().findImage(std::vector<std::size_t>{0, 3}) == 0);
  }

  const VariablePtr C = make_variable(2, "C");
  SECTION("ternary factor") {
    auto factor =
        make_factor({A, B, C}, {{std::vector<std::size_t>{0, 1, 0}, 1.f},
                                {std::vector<std::size_t>{1, 1, 1}, 2.f},
                                {std::vector<std::size_t>{1, 3, 0}, 0.5f},
                                {std::vector<std::size_t>{1, 2, 1}, 0.7f}});

    CHECK(factor.function().findImage(std::vector<std::size_t>{0, 1, 0}) ==
          1.f);
    CHECK(factor.function().findImage(std::vector<std::size_t>{1, 1, 1}) ==
          2.f);
    CHECK(factor.function().findImage(std::vector<std::size_t>{1, 3, 0}) ==
          0.5f);
    CHECK(factor.function().findImage(std::vector<std::size_t>{1, 2, 1}) ==
          0.7f);

    CHECK(factor.function().findImage(std::vector<std::size_t>{0, 2, 0}) == 0);
    CHECK(factor.function().findImage(std::vector<std::size_t>{0, 2, 1}) == 0);
    CHECK(factor.function().findImage(std::vector<std::size_t>{0, 3, 0}) == 0);
    CHECK(factor.function().findImage(std::vector<std::size_t>{0, 3, 1}) == 0);

    SECTION("variables substitution") {
      const VariablesSoup new_vars =
          VariablesSoup{make_variable(3, "A2"), make_variable(4, "B2"),
                        make_variable(2, "C2")};
      factor.replaceVariables(new_vars);
      const auto &vars = factor.function().vars().getVariables();
      CHECK(vars[0]->name() == "A2");
      CHECK(vars[1]->name() == "B2");
      CHECK(vars[2]->name() == "C2");

      const VariablesSoup new_vars_bad =
          VariablesSoup{make_variable(5, "A2"), make_variable(2, "B2"),
                        make_variable(2, "C2")};
      CHECK_THROWS_AS(factor.replaceVariables(new_vars_bad), Error);
    }

    SECTION("exponential wrap evaluation") {
      const float w = 1.5f;
      FactorExponential factor_exp(factor, w);

      CHECK(factor_exp.function().findTransformed(
                std::vector<std::size_t>{0, 1, 0}) == expf(w * 1.f));
      CHECK(factor_exp.function().findTransformed(
                std::vector<std::size_t>{1, 1, 1}) == expf(w * 2.f));
    }
  }
}

TEST_CASE("probabilties computation", "[function]") {
  auto factor =
      make_factor(VariablesSoup{make_variable(2, "A"), make_variable(2, "B")},
                  {{std::vector<std::size_t>{0, 0}, 1.f},
                   {std::vector<std::size_t>{0, 1}, 0.f},
                   {std::vector<std::size_t>{1, 0}, 0.f},
                   {std::vector<std::size_t>{1, 1}, 1.f}});

  SECTION("probabilities") {
    CHECK(test::almost_equal_it(factor.getProbabilities(),
                                std::vector<float>{0.5f, 0, 0, 0.5f}, 0.001f));
  }

  SECTION("exponential wrap probabilities") {
    const float w = 1.5f;
    FactorExponential factor_exp(factor, w);

    const float big = expf(w);
    const float small = 1.f;
    const float cumul = 2.f * (small + big);
    CHECK(test::almost_equal_it(factor_exp.getProbabilities(),
                                std::vector<float>{big / cumul, small / cumul,
                                                   small / cumul, big / cumul},
                                0.001f));
  }
}

#include <EasyFactorGraph/factor/ImageFinder.h>

namespace {
std::vector<std::size_t> make_combination(std::size_t size, std::size_t val) {
  std::vector<std::size_t> result;
  result.reserve(size);
  for (std::size_t k = 0; k < size; ++k) {
    result.push_back(val);
  }
  return result;
}
} // namespace

TEST_CASE("bigger combination finder", "[function]") {
  auto group_size = GENERATE(1, 2, 3);

  VariablesSoup group;
  for (int k = 0; k < group_size; ++k) {
    group.push_back(make_variable(4, "V" + std::to_string(k)));
  }
  Factor factor(Group{group});
  for (std::size_t k = 0; k < 2; ++k) {
    factor.set(make_combination(group_size, k), 1.f);
  }

  VariablesSoup bigger_group;
  for (int k = 0; k < group_size; ++k) {
    bigger_group.push_back(group[k]);
    bigger_group.push_back(make_variable(4, "V" + std::to_string(k) + "_bis"));
  }
  auto factor_finder = factor.makeFinder(bigger_group);

  // 0, ... ,0
  CHECK(factor_finder.findImage(make_combination(bigger_group.size(), 0)) ==
        1.f);
  // 1, ... ,1
  CHECK(factor_finder.findImage(make_combination(bigger_group.size(), 1)) ==
        1.f);
  // 2, ... ,2
  CHECK(factor_finder.findImage(make_combination(bigger_group.size(), 2)) == 0);
  // 3, ... ,3
  CHECK(factor_finder.findImage(make_combination(bigger_group.size(), 3)) == 0);
}

TEST_CASE("bigger combination finder again", "[function]") {
  auto A = make_variable(3, "A");
  auto B = make_variable(3, "B");
  auto C = make_variable(3, "C");
  auto D = make_variable(3, "D");

  VariablesSoup bigger_group = {A, B, C, D};

  Factor factor(Group{VariablesSoup{B, D}});
  factor.set(std::vector<std::size_t>{0, 1}, 1.f);
  factor.set(std::vector<std::size_t>{2, 1}, 2.f);
  factor.set(std::vector<std::size_t>{1, 1}, 3.f);

  auto finder = factor.makeFinder(bigger_group);

  CHECK(finder.findImage(std::vector<std::size_t>{0, 0, 2, 1}) == 1.f);
  CHECK(finder.findImage(std::vector<std::size_t>{2, 2, 0, 1}) == 2.f);
  CHECK(finder.findImage(std::vector<std::size_t>{2, 1, 2, 1}) == 3.f);
}
} // namespace EFG::test
