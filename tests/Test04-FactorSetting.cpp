#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/distribution/Factor.h>

#include <gtest/gtest.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

TEST(DistributionSetting, replaceGroup) {
  VariablesSoup groupAB = {make_variable(3, "A"), make_variable(2, "B")};
  VariablesSoup groupXY = {make_variable(3, "X"), make_variable(2, "Y")};
  VariablesSoup groupXYbis = {make_variable(2, "X"), make_variable(4, "Y")};

  distribution::Factor factor(categoric::Group{groupAB});
  factor.replaceVariables(groupXY);
  ASSERT_THROW(factor.replaceVariables(groupXYbis), EFG::Error);
}

distribution::Factor make_factor_test() {
  VariablesSoup group = {make_variable(3, "A"), make_variable(4, "B"),
                         make_variable(2, "C")};
  return distribution::Factor{Group{group}};
}

void set_image_raw(distribution::Factor &subject,
                   std::vector<std::size_t> &&to_add, const float value) {
  Combination comb(std::move(to_add));
  const std::size_t initial_map_size = subject.getCombinationsMap().size();
  subject.setImageRaw(comb, value);
  EXPECT_EQ(subject.evaluate(comb), value);
  EXPECT_EQ(initial_map_size + 1, subject.getCombinationsMap().size());
}

TEST(DistributionSetting, setImages) {
  auto factor = make_factor_test();

  set_image_raw(factor, {1, 2, 1}, 2.f);
  set_image_raw(factor, {0, 0, 0}, 3.5f);
  set_image_raw(factor, {2, 3, 0}, 1.f);
  set_image_raw(factor, {1, 1, 1}, 0.5f);
  ASSERT_THROW(set_image_raw(factor, {1, 0, 1}, -2.5f), Error);
}

TEST(DistributionSetting, setAllImagesRaw) {
  auto factor = make_factor_test();

  float val = 2.f;
  factor.setAllImagesRaw(val);
  EXPECT_EQ(factor.getCombinationsMap().size(), factor.getVariables().size());
  for (const auto &[comb, comb_val] : factor.getCombinationsMap()) {
    EXPECT_EQ(val, comb_val);
  }

  val = 3.5f;
  factor.setAllImagesRaw(val);
  EXPECT_EQ(factor.getCombinationsMap().size(), factor.getVariables().size());
  for (const auto &[comb, comb_val] : factor.getCombinationsMap()) {
    EXPECT_EQ(val, comb_val);
  }

  ASSERT_THROW(factor.setAllImagesRaw(-1.f), Error);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
