#include <gtest/gtest.h>
#include <CombinationCompare.h>
#include <categoric/Range.h>
#include <distribution/factor/modifiable/Factor.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

class FactorTest 
  : public ::testing::Test
  , public factor::cnst::Factor { 
protected: 
  FactorTest(const std::set<categoric::VariablePtr>& variables)
  : factor::cnst::Factor(variables) { 
  } 

  float getImagesSum() {
    float sum = 0.f;
    for(auto it = this->values->begin(); it!=this->values->end(); ++it) {
      sum += this->evaluator->evaluate(it->second);
    }
    return sum;
  }
};

class FactorUnary : public FactorTest {
public:
  FactorUnary()
    : FactorTest({makeVariable(3, "A")}) {
  };
};

class FactorBinary : public FactorTest {
public:
  FactorBinary()
    : FactorTest({makeVariable(3, "A"), makeVariable(4, "B")}) {
  };
};

class FactorTernary : public FactorTest {
public:
  FactorTernary()
    : FactorTest({makeVariable(3, "A"), makeVariable(4, "B"), makeVariable(2, "C")}) {
  };
};

TEST_F(FactorUnary,findUnary) { 
  std::vector<std::size_t> vals;

  vals = {0}; Combination comb1(vals.data(), vals.size());
  vals = {1}; Combination comb2(vals.data(), vals.size());
  vals = {2}; Combination comb3(vals.data(), vals.size());

  this->values->emplace(comb1, 1.f);
  this->values->emplace(comb3, 2.f);

  EXPECT_EQ(this->find(comb1) , 1.f);
  EXPECT_EQ(this->find(comb2) , 0.f);
  EXPECT_EQ(this->find(comb3) , 2.f);
}

TEST_F(FactorBinary,findBinary) { 
  std::vector<std::size_t> vals;

  vals = {0, 0}; Combination comb1(vals.data(), vals.size());
  vals = {0, 1}; Combination comb2(vals.data(), vals.size());
  vals = {2, 3}; Combination comb3(vals.data(), vals.size());
  vals = {2, 2}; Combination comb4(vals.data(), vals.size());
  vals = {1, 3}; Combination comb5(vals.data(), vals.size());

  this->values->emplace(comb1, 1.f);
  this->values->emplace(comb3, 2.f);
  this->values->emplace(comb5, 3.f);

  EXPECT_EQ(this->find(comb1) , 1.f);
  EXPECT_EQ(this->find(comb2) , 0.f);
  EXPECT_EQ(this->find(comb3) , 2.f);
  EXPECT_EQ(this->find(comb4) , 0.f);
  EXPECT_EQ(this->find(comb5) , 3.f);
}

TEST_F(FactorTernary,findTernary) { 
  std::vector<std::size_t> vals;

  vals = {0, 0, 0}; Combination comb1(vals.data(), vals.size());
  vals = {0, 1, 1}; Combination comb2(vals.data(), vals.size());
  vals = {2, 3, 0}; Combination comb3(vals.data(), vals.size());
  vals = {2, 2, 1}; Combination comb4(vals.data(), vals.size());
  vals = {1, 3, 0}; Combination comb5(vals.data(), vals.size());

  this->values->emplace(comb1, 1.f);
  this->values->emplace(comb3, 2.f);
  this->values->emplace(comb5, 3.f);

  EXPECT_EQ(this->find(comb1) , 1.f);
  EXPECT_EQ(this->find(comb2) , 0.f);
  EXPECT_EQ(this->find(comb3) , 2.f);
  EXPECT_EQ(this->find(comb4) , 0.f);
  EXPECT_EQ(this->find(comb5) , 3.f);
}

// TEST_F(FactorUnary,getProbabilitiesUnary) { 
//   std::vector<std::size_t> vals;

//   vals = {0}; Combination comb1(vals.data(), vals.size());
//   vals = {1}; Combination comb2(vals.data(), vals.size());
//   vals = {2}; Combination comb3(vals.data(), vals.size());

//   this->values->emplace(comb1, 1.f);
//   this->values->emplace(comb3, 2.f);

//   EXPECT_EQ(this->find(comb1) , 1.f);
//   EXPECT_EQ(this->find(comb2) , 0.f);
//   EXPECT_EQ(this->find(comb3) , 2.f);
// }

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
