// #include <EasyFactorGraph/Error.h>
// #include <EasyFactorGraph/categoric/GroupRange.h>
// #include <EasyFactorGraph/distribution/Factor.h>

// #include <gtest/gtest.h>
// using namespace EFG;
// using namespace EFG::categoric;
// using namespace EFG::distribution;

// distribution::Factor make_correlating_factor(const bool correlation_kind) {
//   VariablesSoup group = {make_variable(4, "A"), make_variable(4, "B"),
//                          make_variable(4, "C")};
//   if (correlation_kind) {
//     return distribution::Factor{Group{group}, USE_SIMPLE_CORRELATION_TAG};
//   }
//   return distribution::Factor{Group{group}, USE_SIMPLE_ANTI_CORRELATION_TAG};
// }

// bool all_equals_values(const Combination &comb) {
//   const auto &data = comb.data();
//   for (const auto &val : data) {
//     if (val != data.front()) {
//       return false;
//     }
//   }
//   return true;
// }

// TEST(DistributionMaking, simple_correlation) {
//   auto factor = make_correlating_factor(true);
//   categoric::GroupRange range(factor.getVariables());
//   for_each_combination(range, [&factor](const Combination &comb) {
//     if (all_equals_values(comb)) {
//       EXPECT_EQ(1.f, factor.evaluate(comb));
//     } else {
//       EXPECT_EQ(0, factor.evaluate(comb));
//     }
//   });
// }

// TEST(DistributionMaking, simple_anti_correlation) {
//   auto factor = make_correlating_factor(false);
//   categoric::GroupRange range(factor.getVariables());
//   for_each_combination(range, [&factor](const Combination &comb) {
//     if (all_equals_values(comb)) {
//       EXPECT_EQ(0, factor.evaluate(comb));
//     } else {
//       EXPECT_EQ(1.f, factor.evaluate(comb));
//     }
//   });
// }

// TEST(DistributionMaking, mergeDistributions) {
//   float val1 = 2.f, val2 = 0.5f;

//   distribution::Factor distrAC(
//       Group{{make_variable(2, "A"), make_variable(2, "C")}});
//   distrAC.setAllImagesRaw(val1);
//   distribution::Factor distrBC(
//       Group{{make_variable(2, "B"), make_variable(2, "C")}});
//   distrBC.setAllImagesRaw(val2);

//   distribution::Factor distrABC(distrAC, distrBC);
//   EXPECT_EQ(3, distrABC.getVariables().getVariables().size());
//   {
//     auto it = distrABC.getGroup().getVariables().begin();
//     EXPECT_EQ(std::string("A"), (*it)->name());
//     ++it;
//     EXPECT_EQ(std::string("B"), (*it)->name());
//     ++it;
//     EXPECT_EQ(std::string("C"), (*it)->name());
//   }

//   // std::size_t numberValues = 0;
//   // auto it = distrABC.getIterator();
//   // iterator::forEach(it, [&](const DistributionIterator &it) {
//   //   EXPECT_EQ(it.getImage(), val1 * val2);
//   //   ++numberValues;
//   // });
//   // EXPECT_EQ(numberValues, distrABC.getGroup().size());
// }

// // TEST(DistributionMaking, marginalization) {
// //   VariablePtr A = makeVariable(2, "A");
// //   VariablePtr B = makeVariable(3, "B");
// //   VariablePtr C = makeVariable(2, "C");
// //   VariablePtr D = makeVariable(3, "D");

// //   factor::modif::Factor distrABCD(
// //       std::set<categoric::VariablePtr>({A, B, C, D}));
// //   std::set<VariablePtr> groupBD = {B, D};

// //   std::vector<std::size_t> vals;

// //   Range rangeBD(groupBD);

// //   {
// //     iterator::forEach(rangeBD, [&vals, &distrABCD](const Range &rangeBD) {
// //       vals = std::vector<std::size_t>{1, rangeBD.get().data()[0], 1,
// //                                       rangeBD.get().data()[1]};
// //       distrABCD.setImageRaw(Combination(vals.data(), vals.size()), 1.f);
// //     });
// //   }

// //   {
// //     // marginalization A=0, C=1
// //     vals = {0, 1};
// //     factor::cnst::Factor distrBD(distrABCD,
// //                                  Combination(vals.data(), vals.size()),
// {A,
// //                                  C});
// //     rangeBD.reset();
// //     iterator::forEach(rangeBD, [&distrBD](const Range &rangeBD) {
// //       EXPECT_EQ(distrBD.find(rangeBD.get()), 0.f);
// //     });
// //   }

// //   {
// //     // marginalization A=1, C=1
// //     vals = {1, 1};
// //     factor::cnst::Factor distrBD(distrABCD,
// //                                  Combination(vals.data(), vals.size()),
// {A,
// //                                  C});
// //     rangeBD.reset();
// //     iterator::forEach(rangeBD, [&distrBD](const Range &rangeBD) {
// //       EXPECT_EQ(distrBD.find(rangeBD.get()), 1.f);
// //     });
// //   }
// // }

// // TEST(DistributionMaking, importFromFile) {
// //   factor::cnst::Factor factor(
// //       std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")},
// //       std::string(TEST_FOLDER) + std::string("FactorDescription"));
// // }

// // ////////////////////////////////////////
// // // add test on factor merging
// // ////////////////////////////////////////

// int main(int argc, char *argv[]) {
//   ::testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }
