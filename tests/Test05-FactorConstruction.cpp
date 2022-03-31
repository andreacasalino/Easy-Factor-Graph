#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/Factor.h>

#include <gtest/gtest.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

distribution::Factor make_correlating_factor(const bool correlation_kind) {
  VariablesSoup group = {make_variable(4, "A"), make_variable(4, "B"),
                         make_variable(4, "C")};
  if (correlation_kind) {
    return distribution::Factor{Group{group}, USE_SIMPLE_CORRELATION_TAG};
  }
  return distribution::Factor{Group{group}, USE_SIMPLE_ANTI_CORRELATION_TAG};
}

bool all_equals_values(const Combination &comb) {
  const auto &data = comb.data();
  for (const auto &val : data) {
    if (val != data.front()) {
      return false;
    }
  }
  return true;
}

TEST(DistributionMaking, simple_correlation) {
  auto factor = make_correlating_factor(true);
  categoric::GroupRange range(factor.getVariables());
  for_each_combination(range, [&factor](const Combination &comb) {
    if (all_equals_values(comb)) {
      EXPECT_EQ(1.f, factor.evaluate(comb));
    } else {
      EXPECT_EQ(0, factor.evaluate(comb));
    }
  });
}

TEST(DistributionMaking, simple_anti_correlation) {
  auto factor = make_correlating_factor(false);
  categoric::GroupRange range(factor.getVariables());
  for_each_combination(range, [&factor](const Combination &comb) {
    if (all_equals_values(comb)) {
      EXPECT_EQ(0, factor.evaluate(comb));
    } else {
      EXPECT_EQ(1.f, factor.evaluate(comb));
    }
  });
}

TEST(DistributionMaking, mergeDistributions) {
  float val1 = 2.f, val2 = 0.5f;

  auto varA = make_variable(2, "A");
  auto varB = make_variable(2, "B");
  auto varC = make_variable(2, "C");

  distribution::Factor distrAC(Group{{varA, varC}});
  distrAC.setAllImagesRaw(val1);
  distribution::Factor distrBC(Group{{varB, varC}});
  distrBC.setAllImagesRaw(val2);

  distribution::Factor distrABC(distrAC, distrBC);
  EXPECT_EQ(3, distrABC.getVariables().getVariables().size());
  const auto &distrABC_group = distrABC.getVariables().getVariablesSet();
  EXPECT_TRUE(distrABC_group.find(make_variable(2, "A")) !=
              distrABC_group.end());
  EXPECT_TRUE(distrABC_group.find(make_variable(2, "B")) !=
              distrABC_group.end());
  EXPECT_TRUE(distrABC_group.find(make_variable(2, "C")) !=
              distrABC_group.end());

  GroupRange range(distrABC.getVariables());
  EXPECT_EQ(distrABC.getVariables().size(), 2 * 2 * 2);
  for_each_combination(range, [&](const Combination &comb) {
    EXPECT_EQ(distrABC.evaluate(comb), val1 * val2);
  });
}

// TEST(DistributionMaking, importFromFile) {
//   factor::cnst::Factor factor(
//       std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")},
//       std::string(TEST_FOLDER) + std::string("FactorDescription"));
// }

// ////////////////////////////////////////
// // add test on factor merging
// ////////////////////////////////////////

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
