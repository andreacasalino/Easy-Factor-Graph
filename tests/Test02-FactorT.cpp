#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/SimpleCorrelations.h>
#include <EasyFactorGraph/misc/Generator.h>

#include <Utils.h>

#include <cmath>

namespace EFG::test {
namespace {
struct TwoMultiplier {
  TwoMultiplier() = default;

  float operator()(float val) const { return 2.f * val; }
};

class FunctionFixture : public factor::FactorT<3, TwoMultiplier> {
public:
  FunctionFixture()
      : factor::FactorT<3, TwoMultiplier>{
            factor::FactorT<3, TwoMultiplier>::from_sparse_domain<true>(
                {2, 4, 2}, DOMAIN)} {}

private:
  using Domain = std::vector<std::pair<categoric::Combination<3>, float>>;

  static inline Domain DOMAIN = {
      {{0, 1, 0}, 1.f}, {{1, 3, 0}, 2.f}, {{0, 2, 1}, 3.f}, {{0, 0, 0}, 1.f},
      {{0, 0, 1}, 1.f}, {{0, 2, 0}, 1.f}, {{1, 0, 0}, 2.f}, {{1, 2, 0}, 1.f},
      {{1, 2, 1}, 1.f}, {{1, 3, 1}, 1.f},
  };
};
} // namespace

TEST_CASE("Function get method", "[factorT]") {
  FunctionFixture fnct;

  CHECK(fnct.get<true>({0, 2, 1}) == 2.f * 3.f);
  CHECK(fnct.get<false>({0, 2, 1}) == 3.f);

  CHECK(fnct.get<true>({1, 0, 0}) == 2.f * 2.f);
  CHECK(fnct.get<false>({1, 0, 0}) == 2.f);

  CHECK(fnct.get<true>({1, 0, 1}) == 0);
  CHECK(fnct.get<false>({1, 0, 1}) == 0);
}

TEST_CASE("Function iterate values", "[factorT]") {
  std::vector<std::pair<categoric::Combination<3>, float>> values;
  std::vector<std::pair<categoric::Combination<3>, float>> expected{
      {{0, 0, 0}, 1.f}, {{0, 0, 1}, 1.f}, {{0, 1, 0}, 1.f}, {{0, 1, 1}, 0},
      {{0, 2, 0}, 1.f}, {{0, 2, 1}, 3.f}, {{0, 3, 0}, 0},   {{0, 3, 1}, 0},

      {{1, 0, 0}, 2.f}, {{1, 0, 1}, 0},   {{1, 1, 0}, 0},   {{1, 1, 1}, 0},
      {{1, 2, 0}, 1.f}, {{1, 2, 1}, 1.f}, {{1, 3, 0}, 2.f}, {{1, 3, 1}, 1.f},
  };

  FunctionFixture fnct;
  fnct.forEachCombination<false>([&values](const auto &comb, float val) {
    values.emplace_back(std::make_pair(comb, val));
  });

  CHECK(values == expected);
}

} // namespace EFG::test
