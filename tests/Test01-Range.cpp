#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/categoric/GroupRange.h>

#include <algorithm>

namespace EFG::test {
using namespace categoric;

TEST_CASE("testing for_each", "[range]") {
  VariablePtr A = make_variable(2, "A");
  VariablePtr B = make_variable(2, "B");

  std::vector<std::vector<std::size_t>> expected = {
      {0, 0}, {0, 1}, {1, 0}, {1, 1}};

  std::vector<std::vector<std::size_t>> got;
  std::for_each(GroupRange{{A, B}}, RANGE_END,
                [&got](const auto &comb) { got.emplace_back(comb); });
  CHECK(got == expected);
}

namespace {
std::vector<std::vector<std::size_t>>
all_combinations_in_range(GroupRange &range) {
  std::vector<std::vector<std::size_t>> got;
  for_each_combination(range,
                       [&got](const auto &comb) { got.emplace_back(comb); });
  return got;
}
} // namespace

TEST_CASE("testing for_each_combination", "[range]") {
  SECTION("small binary group") {
    GroupRange rangeAB({make_variable(2, "A"), make_variable(2, "B")});
    CHECK(
        all_combinations_in_range(rangeAB) ==
        std::vector<std::vector<std::size_t>>{{0, 0}, {0, 1}, {1, 0}, {1, 1}});
  }

  SECTION("big binary group") {
    GroupRange rangeAB({make_variable(3, "A"), make_variable(4, "B")});
    CHECK(all_combinations_in_range(rangeAB) ==
          std::vector<std::vector<std::size_t>>{{0, 0},
                                                {0, 1},
                                                {0, 2},
                                                {0, 3},
                                                {1, 0},
                                                {1, 1},
                                                {1, 2},
                                                {1, 3},
                                                {2, 0},
                                                {2, 1},
                                                {2, 2},
                                                {2, 3}});
  }

  SECTION("ternary group") {
    GroupRange rangeAB(
        {make_variable(3, "A"), make_variable(4, "B"), make_variable(2, "C")});
    CHECK(all_combinations_in_range(rangeAB) ==
          std::vector<std::vector<std::size_t>>{
              {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {0, 2, 0},
              {0, 2, 1}, {0, 3, 0}, {0, 3, 1}, {1, 0, 0}, {1, 0, 1},
              {1, 1, 0}, {1, 1, 1}, {1, 2, 0}, {1, 2, 1}, {1, 3, 0},
              {1, 3, 1}, {2, 0, 0}, {2, 0, 1}, {2, 1, 0}, {2, 1, 1},
              {2, 2, 0}, {2, 2, 1}, {2, 3, 0}, {2, 3, 1}});
  }
}
} // namespace EFG::test
