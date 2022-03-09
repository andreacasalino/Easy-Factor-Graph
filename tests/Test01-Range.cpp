#include <gtest/gtest.h>
#include <list>

#include <EasyFactorGraph/categoric/GroupRange.h>

using namespace EFG;
using namespace EFG::categoric;

#include <algorithm>

TEST(Range, binaryGroupSmall_std_foreach) {
  VariablePtr A = make_variable(2, "A");
  VariablePtr B = make_variable(2, "B");

  std::list<std::vector<std::size_t>> expected = {
      {0, 0}, {0, 1}, {1, 0}, {1, 1}};

  std::list<std::vector<std::size_t>> got;
  std::for_each(GroupRange{{A, B}}, RANGE_END, [&got](const Combination &comb) {
    got.emplace_back(comb.data());
  });
  EXPECT_EQ(got, expected);
}

std::list<std::vector<std::size_t>>
all_combinations_in_range(GroupRange &range) {
  std::list<std::vector<std::size_t>> got;
  for_each_combination(range, [&got](const Combination &comb) {
    got.emplace_back(comb.data());
  });
  return got;
}

TEST(Range, binaryGroupSmall) {
  VariablePtr A = make_variable(2, "A");
  VariablePtr B = make_variable(2, "B");
  GroupRange rangeAB({A, B});

  std::list<std::vector<std::size_t>> expected = {
      {0, 0}, {0, 1}, {1, 0}, {1, 1}};

  EXPECT_EQ(all_combinations_in_range(rangeAB), expected);
}

TEST(Range, binaryGroupBig) {
  VariablePtr A = make_variable(3, "A");
  VariablePtr B = make_variable(4, "B");
  GroupRange rangeAB({A, B});

  std::list<std::vector<std::size_t>> expected = {
      {0, 0}, {0, 1}, {0, 2}, {0, 3}, {1, 0}, {1, 1},
      {1, 2}, {1, 3}, {2, 0}, {2, 1}, {2, 2}, {2, 3}};

  EXPECT_EQ(all_combinations_in_range(rangeAB), expected);
}

TEST(Range, ternaryGroup) {
  VariablePtr A = make_variable(3, "A");
  VariablePtr B = make_variable(4, "B");
  VariablePtr C = make_variable(2, "C");
  GroupRange rangeABC({A, B, C});

  std::list<std::vector<std::size_t>> expected = {
      {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {0, 2, 0}, {0, 2, 1},
      {0, 3, 0}, {0, 3, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1},
      {1, 2, 0}, {1, 2, 1}, {1, 3, 0}, {1, 3, 1}, {2, 0, 0}, {2, 0, 1},
      {2, 1, 0}, {2, 1, 1}, {2, 2, 0}, {2, 2, 1}, {2, 3, 0}, {2, 3, 1}};

  EXPECT_EQ(all_combinations_in_range(rangeABC), expected);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
