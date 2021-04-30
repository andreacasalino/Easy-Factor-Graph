#include <gtest/gtest.h>
#include <CombinationCompare.h>
#include <DistributionSamples.h>
#include <distribution/DistributionFinder.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

Combination makeBiggerCombination(const Combination& originalComb, const std::size_t& additions) {
  std::vector<std::size_t> temp;
  temp.reserve(originalComb.size() + additions);
  for(std::size_t k=0; k<originalComb.size(); ++k) {
    temp.push_back(originalComb.data()[k]);
  }
  for(std::size_t k=0; k<additions; ++k) {
    temp.push_back(1);
  }
  return Combination(temp.data(), temp.size());
};

template<typename F>
class DistributionTest : public F {
public:
  DistributionTest() = default;

  void testFinder() const {
    std::set<categoric::VariablePtr> biggerGroup = this->group->getVariables();
    biggerGroup.emplace(makeVariable(3, "Z1"));
    biggerGroup.emplace(makeVariable(3, "Z2"));
    DistributionFinder finder(*this, biggerGroup);
    for(auto it = this->values->begin(); it!=this->values->end(); ++it) {
      Combination comb = makeBiggerCombination(it->first, 2);
      auto res = finder.find(comb);
      test::compare(it->first, *res.first);
      EXPECT_EQ(it->second, res.second);
    }
  }
};

typedef DistributionTest<test::FactorUnaryTest> FactorUnary; 
TEST_F(FactorUnary, findUnary) {  
  this->testFinder();
}

typedef DistributionTest<test::FactorBinaryTest> FactorBinary; 
TEST_F(FactorBinary, findBinary) {  
  this->testFinder();
}

typedef DistributionTest<test::FactorTernaryTest> FactorTernary; 
TEST_F(FactorTernary, findTernary) {  
  this->testFinder();
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
