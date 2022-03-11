#include <gtest/gtest.h>

#include <EasyFactorGraph/distribution/CombinationFinder.h>
#include <EasyFactorGraph/distribution/Factor.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

namespace {
static const VariablePtr A = make_variable(3, "A");
static const VariablePtr B = make_variable(4, "B");
static const VariablePtr C = make_variable(2, "C");
} // namespace

TEST(CombinationFinder, binary_factor) {
  Factor factor({A, B}, USE_SIMPLE_CORRELATION_TAG);

  {
    auto finder = factor.makeFinder({A, B, C});
    EXPECT_EQ(finder.find(Combination{{0, 0, 0}}).value, 1.f);
    EXPECT_EQ(finder.find(Combination{{0, 0, 1}}).value, 1.f);

    EXPECT_EQ(finder.find(Combination{{0, 1, 0}}).value, 0);
    EXPECT_EQ(finder.find(Combination{{0, 1, 1}}).value, 0);
  }
}
