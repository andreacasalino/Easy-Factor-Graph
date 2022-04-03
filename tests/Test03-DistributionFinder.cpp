#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <EasyFactorGraph/distribution/Factor.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

namespace {
categoric::Combination make_combination(const std::size_t size,
                                        const std::size_t val) {
  std::vector<std::size_t> result;
  result.reserve(size);
  for (std::size_t k = 0; k < size; ++k) {
    result.push_back(val);
  }
  return result;
}
} // namespace

TEST_CASE("bigger combination finder", "[distribution]") {
  auto group_size = GENERATE(1, 2, 3);

  VariablesSoup group;
  for (int k = 0; k < group_size; ++k) {
    group.push_back(make_variable(4, "V" + std::to_string(k)));
  }
  Factor factor(Group{group});
  for (std::size_t k = 0; k < 2; ++k) {
    factor.setImageRaw(make_combination(group_size, k), 1.f);
  }

  VariablesSoup bigger_group;
  for (int k = 0; k < group_size; ++k) {
    bigger_group.push_back(group[k]);
    bigger_group.push_back(make_variable(4, "V" + std::to_string(k) + "_bis"));
  }
  auto factor_finder = factor.makeFinder(bigger_group);

  {
    // 0, ... ,0
    auto found = factor_finder.find(make_combination(bigger_group.size(), 0));
    CHECK(found.map_iterator != factor.getCombinationsMap().end());
    CHECK(found.value == 1.f);
  }

  {
    // 1, ... ,1
    auto found = factor_finder.find(make_combination(bigger_group.size(), 1));
    CHECK(found.map_iterator != factor.getCombinationsMap().end());
    CHECK(found.value == 1.f);
  }

  {
    // 2, ... ,2
    auto found = factor_finder.find(make_combination(bigger_group.size(), 2));
    CHECK(found.map_iterator == factor.getCombinationsMap().end());
    CHECK(found.value == 0);
  }

  {
    // 3, ... ,3
    auto found = factor_finder.find(make_combination(bigger_group.size(), 3));
    CHECK(found.map_iterator == factor.getCombinationsMap().end());
    CHECK(found.value == 0);
  }
}
