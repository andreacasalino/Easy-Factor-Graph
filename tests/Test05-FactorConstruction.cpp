#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/Factor.h>

using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

namespace {
bool all_equals_values(const Combination &comb) {
  const auto &data = comb.data();
  for (const auto &val : data) {
    if (val != data.front()) {
      return false;
    }
  }
  return true;
}
} // namespace

TEST_CASE("correlating factors", "[factor]") {
  Group group(VariablesSoup{make_variable(4, "A"), make_variable(4, "B"),
                            make_variable(4, "C")});

  SECTION("correlating factor") {
    Factor factor(group, USE_SIMPLE_CORRELATION_TAG);
    categoric::GroupRange range(factor.getVariables());
    for_each_combination(range, [&factor](const Combination &comb) {
      if (all_equals_values(comb)) {
        CHECK(1.f == factor.evaluate(comb));
      } else {
        CHECK(0 == factor.evaluate(comb));
      }
    });
  }

  SECTION("anti correlating factor") {
    Factor factor(group, USE_SIMPLE_ANTI_CORRELATION_TAG);
    categoric::GroupRange range(factor.getVariables());
    for_each_combination(range, [&factor](const Combination &comb) {
      if (all_equals_values(comb)) {
        CHECK(0 == factor.evaluate(comb));
      } else {
        CHECK(1.f == factor.evaluate(comb));
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

  distribution::Factor distrAC(Group{{varA, varC}});
  distrAC.setAllImagesRaw(val1);
  distribution::Factor distrBC(Group{{varB, varC}});
  distrBC.setAllImagesRaw(val2);

  distribution::Factor distrABC(distrAC, distrBC);
  REQUIRE(3 == distrABC.getVariables().getVariables().size());

  const auto &distrABC_group = distrABC.getVariables().getVariablesSet();
  REQUIRE(distrABC_group.find(make_variable(2, "A")) != distrABC_group.end());
  REQUIRE(distrABC_group.find(make_variable(2, "B")) != distrABC_group.end());
  REQUIRE(distrABC_group.find(make_variable(2, "C")) != distrABC_group.end());

  GroupRange range(distrABC.getVariables());
  REQUIRE(distrABC.getVariables().size() == 2 * 2 * 2);
  for_each_combination(range, [&](const Combination &comb) {
    CHECK(distrABC.evaluate(comb) == val1 * val2);
  });
}
