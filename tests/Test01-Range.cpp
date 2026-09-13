#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/categoric/Range.h>
#include <EasyFactorGraph/misc/Generator.h>

#include <algorithm>
#include <vector>

namespace EFG::test {
using namespace categoric;

namespace {
template <std::size_t N>
std::vector<Combination<N>> materialize(Range<N> &&range) {
  std::vector<Combination<N>> res;
  misc::for_each_generated<const Combination<N> *>(
      std::forward<Range<N>>(range),
      [&res](const auto *comb) { res.emplace_back(*comb); });
  return res;
}
} // namespace

TEST_CASE("testing for_each_combination small binary group", "[range]") {
  auto got = materialize(Range<2>{{2, 2}});
  auto expected = std::vector<Combination<2>>{{0, 0}, {0, 1}, {1, 0}, {1, 1}};
  CHECK(got == expected);
}

TEST_CASE("testing for_each_combination big binary group", "[range]") {
  auto got = materialize(Range<2>{{3, 4}});
  auto expected = std::vector<Combination<2>>{{0, 0}, {0, 1}, {0, 2}, {0, 3},
                                              {1, 0}, {1, 1}, {1, 2}, {1, 3},
                                              {2, 0}, {2, 1}, {2, 2}, {2, 3}};
  CHECK(got == expected);
}

TEST_CASE("testing for_each_combination ternary group", "[range]") {
  auto got = materialize(Range<3>{{3, 4, 2}});
  auto expected = std::vector<Combination<3>>{
      {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1}, {0, 2, 0}, {0, 2, 1},
      {0, 3, 0}, {0, 3, 1}, {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1},
      {1, 2, 0}, {1, 2, 1}, {1, 3, 0}, {1, 3, 1}, {2, 0, 0}, {2, 0, 1},
      {2, 1, 0}, {2, 1, 1}, {2, 2, 0}, {2, 2, 1}, {2, 3, 0}, {2, 3, 1}};
  CHECK(got == expected);
}
} // namespace EFG::test
