#include <gtest/gtest.h>
#include <CombinationCompare.h>
#include <distribution/factor/modifiable/Factor.h>
#include <Error.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

TEST(DistributionSetting, replaceGroup) {  
  std::set<VariablePtr> groupAB = {makeVariable(3, "A"), makeVariable(2 , "B")};
  std::set<VariablePtr> groupXY = {makeVariable(3, "X"), makeVariable(2 , "Y")};
  std::set<VariablePtr> groupXYbis = {makeVariable(2, "X"), makeVariable(4 , "Y")};

  factor::modif::Factor factor(groupAB);
  factor.replaceGroup(Group(groupXY));
  ASSERT_THROW(factor.replaceGroup(Group(groupXYbis)), EFG::Error);
}

class FactorTest 
: public ::testing::Test
, public factor::modif::Factor { 
public: 
    FactorTest()
      : factor::modif::Factor(std::set<VariablePtr>{makeVariable(3, "A"), makeVariable(4 , "B"), makeVariable(2 , "C")}) {
    };
};

TEST_F(FactorTest, setImages) {  
  std::vector<std::size_t> combRaw;
  float val;  
  {
    combRaw = {1,2,1};
    val = 2.f;
    Combination comb(combRaw.data(), combRaw.size());
    std::size_t valuesSize = this->values->size();
    this->setImageRaw(comb, val);
    EXPECT_EQ(valuesSize + 1, this->values->size());
    EXPECT_EQ(this->values->find(comb)->second, val);
  }
  {
    combRaw = {0,0,0};
    val = 3.5f;
    Combination comb(combRaw.data(), combRaw.size());
    std::size_t valuesSize = this->values->size();
    this->setImageRaw(comb, val);
    EXPECT_EQ(valuesSize + 1, this->values->size());
    EXPECT_EQ(this->values->find(comb)->second, val);
  }
  {
    combRaw = {2, 3, 0};
    val = 1.f;
    Combination comb(combRaw.data(), combRaw.size());
    std::size_t valuesSize = this->values->size();
    this->setImageRaw(comb, val);
    EXPECT_EQ(valuesSize + 1, this->values->size());
    EXPECT_EQ(this->values->find(comb)->second, val);
  }

  {
    combRaw = {1, 1, 1};
    val = -5.f;
    Combination comb(combRaw.data(), combRaw.size());
    std::size_t valuesSize = this->values->size();
    ASSERT_THROW(this->setImageRaw(comb, val), Error);
  }
}

TEST_F(FactorTest, fillDomain) {
  std::map<Combination, float> initialCombinations;

  std::vector<std::size_t> combRaw;
  {
    combRaw = {1,2,1};
    initialCombinations.emplace(Combination(combRaw.data(), combRaw.size()), 1.f);
  }
  {
    combRaw = {0,0,0};
    initialCombinations.emplace(Combination(combRaw.data(), combRaw.size()), 0.5f);
  }
  {
    combRaw = {2,3,0};
    initialCombinations.emplace(Combination(combRaw.data(), combRaw.size()), 2.3f);
  }

  for(auto it = initialCombinations.begin(); it!=initialCombinations.end(); ++it) {
    this->values->emplace(it->first, it->second);
  }

  this->fillDomain();
  EXPECT_EQ(this->values->size(), this->group->size());

  for(auto it =this->values->begin(); it!=this->values->end(); ++it) {
    auto itInitial = initialCombinations.find(it->first);
    if(itInitial != initialCombinations.end()) {
      EXPECT_EQ(it->second, itInitial->second);
    } 
    else {
      EXPECT_EQ(it->second, 0.f);
    }
  }
}

TEST_F(FactorTest, setAllImagesRaw) {
  float val = 2.f;  
  this->setAllImagesRaw(val);
  EXPECT_EQ(this->values->size(), this->group->size());
  for(auto it = this->values->begin(); it!=this->values->end(); ++it) {
    EXPECT_EQ(it->second, val);
  }

  val = 3.5f;  
  this->setAllImagesRaw(val);
  EXPECT_EQ(this->values->size(), this->group->size());
  for(auto it = this->values->begin(); it!=this->values->end(); ++it) {
    EXPECT_EQ(it->second, val);
  }

  ASSERT_THROW(this->setAllImagesRaw(-1.f), Error);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
