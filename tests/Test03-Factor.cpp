#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/FactorExponential.h>

#include "../src/src/io/Utils.h"
#include "Utils.h"
#include <fstream>
#include <math.h>

namespace EFG::test {
using namespace categoric;
using namespace factor;
using namespace train;

namespace {
bool set_image(factor::Factor &subject, const std::vector<std::size_t> &to_add,
               float value) {
  subject.set(to_add, value);
  return subject.function().findImage(to_add) == value;
}
} // namespace

TEST_CASE("operations on factor", "[factor]") {
  Factor factor(Group{
      {make_variable(3, "A"), make_variable(4, "B"), make_variable(2, "C")}});

  CHECK(set_image(factor, {1, 2, 1}, 2.f));
  CHECK(set_image(factor, {0, 0, 0}, 3.5f));
  CHECK(set_image(factor, {2, 3, 0}, 1.f));
  CHECK(set_image(factor, {1, 1, 1}, 0.5f));

  CHECK_THROWS_AS(set_image(factor, {1, 0, 1}, -2.5f), Error);
}

TEST_CASE("import from file", "[factor]") {
  const std::string file_name = "factor_description";
  {
    std::ofstream stream(file_name);
    if (!stream.is_open()) {
      throw Error::make(file_name, " is an invalid filename");
    }
    stream << "0 1 1 2.0\n";
    stream << "0 0 0 3.0\n";
    stream << "1 1 3 2.5\n";
    stream << "1 0 2 1.4";
  }
  // 2,2,4
  factor::Factor factor(test::make_group(std::vector<std::size_t>{2, 2, 4}));

  io::import_values(factor, file_name);

  CHECK(test::almost_equal(
      factor.function().findImage(std::vector<std::size_t>{0, 1, 1}), 2.f,
      0.01f));
  CHECK(test::almost_equal(
      factor.function().findImage(std::vector<std::size_t>{0, 0, 0}), 3.f,
      0.01f));
  CHECK(test::almost_equal(
      factor.function().findImage(std::vector<std::size_t>{1, 1, 3}), 2.5f,
      0.01f));
  CHECK(test::almost_equal(
      factor.function().findImage(std::vector<std::size_t>{1, 0, 2}), 1.4f,
      0.01f));
}

#include <EasyFactorGraph/categoric/GroupRange.h>

namespace {
bool all_equals_values(const std::vector<std::size_t> &comb) {
  return std::all_of(comb.begin() + 1, comb.end(),
                     [val = comb.front()](auto el) { return val == el; });
}
} // namespace

TEST_CASE("correlating factors", "[factor]") {
  auto sizes = std::vector<std::size_t>{4, 4, 4};

  SECTION("correlating factor") {
    Factor factor(test::make_group(sizes),
                  factor::Factor::SimplyCorrelatedTag{});
    categoric::GroupRange range(factor.function().vars());
    for_each_combination(range,
                         [&function = factor.function()](const auto &comb) {
                           if (all_equals_values(comb)) {
                             CHECK(1.f == function.findImage(comb));
                           } else {
                             CHECK(0 == function.findImage(comb));
                           }
                         });
  }

  SECTION("anti correlating factor") {
    Factor factor(test::make_group(sizes),
                  factor::Factor::SimplyAntiCorrelatedTag{});
    categoric::GroupRange range(factor.function().vars());
    for_each_combination(range,
                         [&function = factor.function()](const auto &comb) {
                           if (all_equals_values(comb)) {
                             CHECK(0 == function.findImage(comb));
                           } else {
                             CHECK(1.f == function.findImage(comb));
                           }
                         });
  }
}

TEST_CASE("merge factors", "[factor]") {
  const float val1 = 1.5f;
  const float val2 = 3.2f;

  auto varA = make_variable(2, "A");
  auto varB = make_variable(2, "B");
  auto varC = make_variable(2, "C");

  factor::Factor distrAC(Group{{varA, varC}});
  test::setAllImages(distrAC, val1);
  factor::Factor distrBC(Group{{varB, varC}});
  test::setAllImages(distrBC, val2);

  factor::Factor distrABC(std::vector<const Immutable *>{&distrAC, &distrBC});
  REQUIRE(3 == distrABC.function().vars().getVariables().size());

  const auto &distrABC_group = distrABC.function().vars().getVariablesSet();
  REQUIRE(distrABC_group.find(make_variable(2, "A")) != distrABC_group.end());
  REQUIRE(distrABC_group.find(make_variable(2, "B")) != distrABC_group.end());
  REQUIRE(distrABC_group.find(make_variable(2, "C")) != distrABC_group.end());

  GroupRange range(distrABC.function().vars());
  REQUIRE(distrABC.function().vars().size() == 2 * 2 * 2);
  for_each_combination(range, [&](const auto &comb) {
    CHECK(distrABC.function().findImage(comb) == val1 * val2);
  });
}

TEST_CASE("Factor copy c'tor", "[factor]") {
  Group group(VariablesSoup{make_variable(4, "A"), make_variable(4, "B"),
                            make_variable(4, "C")});

  Factor factor(test::make_group(std::vector<std::size_t>{4, 4, 4}),
                factor::Factor::SimplyCorrelatedTag{});

  SECTION("Factor") {
    Factor copy(factor);
    CHECK(test::almost_equal_fnct(factor.function(), copy.function()));
  }

  SECTION("FactorExponential") {
    const float w = 1.3f;
    FactorExponential factor_exp(factor, w);
    FactorExponential copy(factor_exp);
    CHECK(test::almost_equal_fnct(factor_exp.function(), copy.function()));
  }

  SECTION("Factor from FactorExponential") {
    const float w = 1.3f;
    FactorExponential factor_exp(factor, w);
    Factor copy(factor_exp, factor::Factor::CloneTrasformedImagesTag{});
    CHECK(test::almost_equal_fnct(factor_exp.function(), copy.function()));
  }
}

TEST_CASE("Variables order change in factor", "[factor]") {
  auto A = make_variable(3, "A");
  auto B = make_variable(3, "B");
  auto C = make_variable(3, "C");

  Group initial_group(VariablesSoup{A, B, C});
  Factor initial_order(initial_group);
  initial_order.set(std::vector<std::size_t>{0, 1, 2}, 1.f);
  initial_order.set(std::vector<std::size_t>{0, 2, 0}, 2.f);
  initial_order.set(std::vector<std::size_t>{1, 0, 1}, 3.f);

  Group different_group(VariablesSoup{B, C, A});
  auto different_order = initial_order.cloneWithPermutedGroup(different_group);

  REQUIRE(different_order.function().vars() == different_group);
  CHECK(different_order.function().findImage(
            std::vector<std::size_t>{1, 2, 0}) == 1.f);
  CHECK(different_order.function().findImage(
            std::vector<std::size_t>{2, 0, 0}) == 2.f);
  CHECK(different_order.function().findImage(
            std::vector<std::size_t>{0, 1, 1}) == 3.f);
}

} // namespace EFG::test
