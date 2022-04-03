// #include <gtest/gtest.h>

// #include <EasyFactorGraph/Error.h>
// #include <EasyFactorGraph/distribution/Factor.h>
// #include <EasyFactorGraph/distribution/FactorExponential.h>
// using namespace EFG;
// using namespace EFG::categoric;
// using namespace EFG::distribution;

// Factor make_factor(const VariablesSoup &vars,
//                    const CombinationRawValuesMap &map) {
//   Factor result(Group{vars});
//   for (const auto &[comb, val] : map) {
//     result.setImageRaw(comb, val);
//   }
//   return result;
// }

// namespace {
// static const VariablePtr A = make_variable(3, "A");
// static const VariablePtr B = make_variable(4, "B");
// static const VariablePtr C = make_variable(2, "C");
// } // namespace

// TEST(DistributionQuery, unary_factor) {
//   auto factor = make_factor({A}, {{std::vector<std::size_t>{0}, 1.f},
//                                   {std::vector<std::size_t>{1}, 2.f}});

//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0}), 1.f);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{1}), 2.f);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{2}), 0);
// }

// TEST(DistributionQuery, binary_factor) {
//   auto factor = make_factor({A, B}, {{std::vector<std::size_t>{0, 1}, 1.f},
//                                      {std::vector<std::size_t>{1, 1}, 2.f},
//                                      {std::vector<std::size_t>{1, 3}, 0.5f},
//                                      {std::vector<std::size_t>{1, 2},
//                                      0.7f}});

//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0, 1}), 1.f);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{1, 1}), 2.f);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{1, 3}), 0.5f);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{1, 2}), 0.7f);

//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0, 2}), 0);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0, 3}), 0);
// }

// Factor make_ternary_factor() {
//   return make_factor({A, B, C}, {{std::vector<std::size_t>{0, 1, 0}, 1.f},
//                                  {std::vector<std::size_t>{1, 1, 1}, 2.f},
//                                  {std::vector<std::size_t>{1, 3, 0}, 0.5f},
//                                  {std::vector<std::size_t>{1, 2, 1}, 0.7f}});
// }

// #include <math.h>

// TEST(DistributionQuery, ternary_factor) {
//   auto factor = make_ternary_factor();

//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0, 1, 0}), 1.f);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{1, 1, 1}), 2.f);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{1, 3, 0}), 0.5f);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{1, 2, 1}), 0.7f);

//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0, 2, 0}), 0);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0, 2, 1}), 0);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0, 3, 0}), 0);
//   EXPECT_EQ(factor.evaluate(std::vector<std::size_t>{0, 3, 1}), 0);

//   const float w = 1.5f;
//   FactorExponential factor_exp(factor, w);
//   EXPECT_EQ(factor_exp.evaluate(std::vector<std::size_t>{0, 1, 0}),
//             expf(w * 1.f));
//   EXPECT_EQ(factor_exp.evaluate(std::vector<std::size_t>{1, 1, 1}),
//             expf(w * 2.f));
// }

// TEST(DistributionQuery, ternary_factor_vars_change) {
//   auto factor = make_ternary_factor();
//   const VariablesSoup new_vars = VariablesSoup{
//       make_variable(3, "A2"), make_variable(4, "B2"), make_variable(2,
//       "C2")};
//   factor.replaceVariables(new_vars);
//   EXPECT_EQ(factor.getVariables().getVariables()[0]->name(), "A2");
//   EXPECT_EQ(factor.getVariables().getVariables()[1]->name(), "B2");
//   EXPECT_EQ(factor.getVariables().getVariables()[2]->name(), "C2");

//   const VariablesSoup new_vars_bad = VariablesSoup{
//       make_variable(5, "A2"), make_variable(2, "B2"), make_variable(2,
//       "C2")};
//   EXPECT_THROW(factor.replaceVariables(new_vars_bad), Error);
// }

// #include <EasyFactorGraph/categoric/GroupRange.h>

// TEST(DistributionQuery, ternary_factor_set_ones) {
//   auto factor = make_ternary_factor();
//   factor.setAllImagesRaw(1.f);
//   const auto &factor_map = factor.getCombinationsMap();
//   EXPECT_EQ(factor_map.size(), factor.getVariables().size());
//   GroupRange range(factor.getVariables());
//   for_each_combination(range, [&factor_map](const Combination &comb) {
//     auto factor_map_it = factor_map.find(comb);
//     EXPECT_TRUE(factor_map_it != factor_map.end());
//     EXPECT_EQ(factor_map_it->second, 1.f);
//   });
// }

// bool are_similars(const std::vector<float> &a, const std::vector<float> &b) {
//   if (a.size() != b.size()) {
//     return false;
//   }
//   for (std::size_t k = 0; k < a.size(); ++k) {
//     if (abs(a[k] - b[k]) > 1e-2) {
//       return false;
//     }
//   }
//   return true;
// }

// TEST(DistributionQuery, binary_factor_probability) {
//   auto factor = make_factor({make_variable(2, "A"), make_variable(2, "B")},
//                             {{std::vector<std::size_t>{0, 0}, 1.f},
//                              {std::vector<std::size_t>{0, 1}, 0},
//                              {std::vector<std::size_t>{1, 0}, 0},
//                              {std::vector<std::size_t>{1, 1}, 1.f}});

//   EXPECT_TRUE(are_similars(factor.getProbabilities(), {0.5, 0, 0, 0.5}));

//   const float w = 1.3f;
//   FactorExponential factor_exp(factor, w);

//   {
//     const float big = expf(w);
//     const float small = 1.f;
//     const float cumul = 2.f * (small + big);
//     std::vector<float> expected = {big / cumul, small / cumul, small / cumul,
//                                    big / cumul};
//     EXPECT_TRUE(are_similars(factor_exp.getProbabilities(), expected));
//   }
// }

// int main(int argc, char *argv[]) {
//   ::testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }
