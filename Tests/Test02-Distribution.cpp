#include <gtest/gtest.h>
#include <CombinationCompare.h>
#include <DistributionSamples.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

template<typename F>
class DistributionTest : public F {
public:
  DistributionTest() = default;

  float getImagesSum() const {
    float sum = 0.f;
    for(auto it = this->values->begin(); it!=this->values->end(); ++it) {
      sum += this->evaluator->evaluate(it->second);
    }
    return sum;
  }

  void compare(const std::vector<float>& probs) const {
    EXPECT_EQ(probs.size(), this->combinations.size());
    float imageSum = this->getImagesSum();
    for(std::size_t k=0; k<probs.size(); ++k) {
      EXPECT_EQ(probs[k], this->find(this->combinations[k]) / imageSum);
    }
  };
};

typedef DistributionTest<test::FactorUnaryTest> DistributionUnary; 
typedef DistributionTest<test::FactorBinaryTest> DistributionBinary; 
typedef DistributionTest<test::FactorTernaryTest> DistributionTernary; 

TEST_F(DistributionUnary, findUnary) { 
  EXPECT_EQ(this->find(combinations[0]) , 1.f);
  EXPECT_EQ(this->find(combinations[1]) , 0.f);
  EXPECT_EQ(this->find(combinations[2]) , 2.f);
}

TEST_F(DistributionBinary,findBinary) { 
  EXPECT_EQ(this->find(combinations[0]) , 1.f);
  EXPECT_EQ(this->find(combinations[1]) , 0.f);
  EXPECT_EQ(this->find(combinations[2]) , 2.f);
  EXPECT_EQ(this->find(combinations[3]) , 0.f);
  EXPECT_EQ(this->find(combinations[4]) , 3.f);
}

TEST_F(DistributionTernary,findTernary) { 
  EXPECT_EQ(this->find(combinations[0]) , 1.f);
  EXPECT_EQ(this->find(combinations[1]) , 0.f);
  EXPECT_EQ(this->find(combinations[2]) , 2.f);
  EXPECT_EQ(this->find(combinations[3]) , 0.f);
  EXPECT_EQ(this->find(combinations[4]) , 3.f);
  EXPECT_EQ(this->find(combinations[6]) , 1.5f);
}

TEST_F(DistributionUnary,getProbabilitiesUnary) { 
  this->compare(this->getProbabilities());
}

TEST_F(DistributionBinary,getProbabilitiesBinary) {  
  this->compare(this->getProbabilities());
}

TEST_F(DistributionTernary,getProbabilitiesBinary) {  
  this->compare(this->getProbabilities());
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
