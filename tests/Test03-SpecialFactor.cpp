#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/structure/SpecialFactors.h>

#include "Utils.h"

namespace EFG::test {
using namespace categoric;
using namespace factor;

TEST_CASE("Unary factor ones at init", "[factor-special]") {
  auto var = make_variable(3, "A");
  MergedUnaries factor(var);
  REQUIRE(factor.function().vars().getVariables() == VariablesSoup{var});
  std::vector<float> imgs;
  factor.function().forEachCombination<false>(
      [&imgs](const auto &, float img) { imgs.push_back(img); });
  CHECK(imgs == std::vector<float>{1.f, 1.f, 1.f});
}

TEST_CASE("Unary factor merge factors", "[factor-special]") {
  auto var = make_variable(3, "A");

  Factor distr_1(Group{var});
  test::setAllImages(distr_1, 1.f);
  distr_1.set(std::vector<std::size_t>{0}, 0.1f);

  Factor distr_2(Group{var});
  test::setAllImages(distr_2, 1.f);
  distr_1.set(std::vector<std::size_t>{1}, 0.2f);

  Factor distr_3(Group{var});
  test::setAllImages(distr_3, 1.f);
  distr_1.set(std::vector<std::size_t>{2}, 0.3f);

  MergedUnaries factor(
      std::vector<const Immutable *>{&distr_1, &distr_2, &distr_3});

  REQUIRE(factor.function().vars().getVariables() == VariablesSoup{var});

  factor::Function expected_distr{Group{var}};
  expected_distr.set(std::vector<std::size_t>{0}, 0.1f / 0.3f);
  expected_distr.set(std::vector<std::size_t>{1}, 0.2f / 0.3f);
  expected_distr.set(std::vector<std::size_t>{2}, 0.3f / 0.3f);
  CHECK(test::almost_equal_fnct(factor.function(), expected_distr));
}

TEST_CASE("Indicator", "[factor-special]") {
  auto var = make_variable(3, "A");
  Indicator factor(var, 1);

  factor::Function expected_distr{Group{var}};
  expected_distr.set(std::vector<std::size_t>{0}, 0);
  expected_distr.set(std::vector<std::size_t>{1}, 1.f);
  expected_distr.set(std::vector<std::size_t>{2}, 0);
  CHECK(test::almost_equal_fnct(factor.function(), expected_distr));
}

namespace {
auto make_exp_test_group() {
  return test::make_group(std::vector<std::size_t>{
      2,
      2,
  });
}

FactorExponential make_exp_test_factor(float w) {
  return FactorExponential{
      Factor{make_exp_test_group(), Factor::SimplyCorrelatedTag{}}, w};
}
} // namespace

TEST_CASE("Evidence", "[factor-special]") {
  const float w = 1.3f;
  auto factor = make_exp_test_factor(w);

  Evidence evidence(factor, factor.function().vars().getVariables()[0], 1);

  CHECK(evidence.function().vars().getVariables().front() ==
        factor.function().vars().getVariables()[1]);
  factor::Function expected_distr{ Group{make_variable(2, "A")}};
  expected_distr.set(std::vector<std::size_t>{0}, 1.f);
  expected_distr.set(std::vector<std::size_t>{1}, exp(w));
  CHECK(test::almost_equal_fnct(evidence.function(), expected_distr));
}

TEST_CASE("Message", "[factor-special]") {
  const float w = 1.3f;
  const float g = 0.6f;

  auto factor_AB = make_exp_test_factor(w);
  auto A = factor_AB.function().vars().getVariables().front();
  auto B = factor_AB.function().vars().getVariables().back();

  Factor shape_A(Group{A});
  shape_A.set(std::vector<std::size_t>{0}, 0.5f);
  shape_A.set(std::vector<std::size_t>{1}, 1.f);
  FactorExponential factor_A(shape_A, g);

  UnaryFactor factor_A_normalized =
      MergedUnaries{std::vector<const Immutable *>{&factor_A}};

  std::vector<float> values;
  factor_A_normalized.function().forEachCombination<true>(
      [&values](const auto &, float img) { values.push_back(img); });

  SECTION("SUM") {
    MessageSUM message(factor_A_normalized, factor_AB);

    factor::Function expected_distr{Group{B}};
    expected_distr.set(std::vector<std::size_t>{0},
                       exp(w) * values.front() + values.back());
    expected_distr.set(std::vector<std::size_t>{1},
                       values.front() + exp(w) * values.back());
    CHECK(test::almost_equal_fnct(message.function(), expected_distr));
  }

  SECTION("MAP") {
    MessageMAP message(factor_A_normalized, factor_AB);

    factor::Function expected_distr{Group{B}};
    expected_distr.set(std::vector<std::size_t>{0},
                       std::max<float>(exp(w) * values.front(), values.back()));
    expected_distr.set(std::vector<std::size_t>{1},
                       std::max<float>(values.front(), exp(w) * values.back()));
    CHECK(test::almost_equal_fnct(message.function(), expected_distr));
  }
}
} // namespace EFG::test
