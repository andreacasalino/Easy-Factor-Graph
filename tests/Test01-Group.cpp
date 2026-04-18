#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/categoric/Group.h>

namespace EFG::test {
using namespace categoric;

TEST_CASE("testing combinationsLen", "[group]") {
  Group<3> group{{2, 3, 2}};

  CHECK(group.combinationsLen() == 2 * 3 * 2);
}

TEST_CASE("testing combinationIndex", "[group]") {
  Group<3> group{{2, 3, 2}};

  CHECK(group.combinationIndex({0, 0, 0}) == 0);
  CHECK(group.combinationIndex({0, 0, 1}) == 1);
  CHECK(group.combinationIndex({0, 1, 0}) == 2);
  CHECK(group.combinationIndex({0, 1, 1}) == 3);
  CHECK(group.combinationIndex({0, 2, 0}) == 4);
  CHECK(group.combinationIndex({0, 2, 1}) == 5);

  CHECK(group.combinationIndex({1, 0, 0}) == 6);
  CHECK(group.combinationIndex({1, 0, 1}) == 7);
  CHECK(group.combinationIndex({1, 1, 0}) == 8);
  CHECK(group.combinationIndex({1, 1, 1}) == 9);
  CHECK(group.combinationIndex({1, 2, 0}) == 10);
  CHECK(group.combinationIndex({1, 2, 1}) == 11);
}

TEST_CASE("testing combinationFromIndex", "[group]") {
  Group<3> group{{2, 3, 2}};

  CHECK(group.combinationFromIndex(0) == Combination<3>{0, 0, 0});
  CHECK(group.combinationFromIndex(1) == Combination<3>{0, 0, 1});
  CHECK(group.combinationFromIndex(2) == Combination<3>{0, 1, 0});
  CHECK(group.combinationFromIndex(3) == Combination<3>{0, 1, 1});
  CHECK(group.combinationFromIndex(4) == Combination<3>{0, 2, 0});
  CHECK(group.combinationFromIndex(5) == Combination<3>{0, 2, 1});

  CHECK(group.combinationFromIndex(6) == Combination<3>{1, 0, 0});
  CHECK(group.combinationFromIndex(7) == Combination<3>{1, 0, 1});
  CHECK(group.combinationFromIndex(8) == Combination<3>{1, 1, 0});
  CHECK(group.combinationFromIndex(9) == Combination<3>{1, 1, 1});
  CHECK(group.combinationFromIndex(10) == Combination<3>{1, 2, 0});
  CHECK(group.combinationFromIndex(11) == Combination<3>{1, 2, 1});
}
} // namespace EFG::test
