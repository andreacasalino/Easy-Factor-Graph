// #include <gtest/gtest.h>

// #include <EasyFactorGraph/distribution/CombinationFinder.h>
// #include <EasyFactorGraph/distribution/Factor.h>
// using namespace EFG;
// using namespace EFG::categoric;
// using namespace EFG::distribution;

// categoric::Combination make_combination(const std::size_t size,
//                                         const std::size_t val) {
//   std::vector<std::size_t> result;
//   result.reserve(size);
//   for (std::size_t k = 0; k < size; ++k) {
//     result.push_back(val);
//   }
//   return result;
// }

// distribution::Factor make_factor(const std::size_t group_size) {
//   categoric::VariablesSoup group;
//   for (std::size_t k = 0; k < group_size; ++k) {
//     const std::string name = "V" + std::to_string(k);
//     group.push_back(categoric::make_variable(4, name));
//   }
//   distribution::Factor result(categoric::Group{group});
//   for (std::size_t k = 0; k < 2; ++k) {
//     result.setImageRaw(make_combination(group_size, k), 1.f);
//   }
//   return result;
// }

// void check_finder(const distribution::Distribution &subject) {
//   categoric::VariablesSoup bigger_group;
//   {
//     auto temp = subject.getVariables().getVariablesSet();
//     temp.emplace(categoric::make_variable(4, "V0_0"));
//     temp.emplace(categoric::make_variable(4, "V1_0"));
//     bigger_group = categoric::VariablesSoup{temp.begin(), temp.end()};
//   }
//   auto finder = subject.makeFinder(bigger_group);
//   {
//     // 0, ... ,0
//     auto val = finder.find(make_combination(bigger_group.size(), 0));
//     EXPECT_TRUE(val.map_iterator != subject.getCombinationsMap().end());
//     EXPECT_EQ(val.value, 1.f);
//   }
//   {
//     // 1, ... ,1
//     auto val = finder.find(make_combination(bigger_group.size(), 1));
//     EXPECT_TRUE(val.map_iterator != subject.getCombinationsMap().end());
//     EXPECT_EQ(val.value, 1.f);
//   }
//   {
//     // 2, ... ,2
//     auto val = finder.find(make_combination(bigger_group.size(), 2));
//     EXPECT_EQ(val.map_iterator, subject.getCombinationsMap().end());
//     EXPECT_EQ(val.value, 0);
//   }
//   {
//     // 3, ... ,3
//     auto val = finder.find(make_combination(bigger_group.size(), 3));
//     EXPECT_EQ(val.map_iterator, subject.getCombinationsMap().end());
//     EXPECT_EQ(val.value, 0);
//   }
// }

// TEST(CombinationFinder, unary_factor) {
//   auto factor = make_factor(1);
//   check_finder(factor);
// }

// TEST(CombinationFinder, binary_factor) {
//   auto factor = make_factor(2);
//   check_finder(factor);
// }

// TEST(CombinationFinder, ternary_factor) {
//   auto factor = make_factor(3);
//   check_finder(factor);
// }

// int main(int argc, char *argv[]) {
//   ::testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }
