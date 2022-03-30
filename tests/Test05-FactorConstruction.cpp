#include <CombinationCompare.h>
#include <Error.h>
#include <categoric/Range.h>
#include <distribution/DistributionIterator.h>
#include <distribution/factor/modifiable/Factor.h>
#include <gtest/gtest.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;

class FactorCorrelationTest : public ::testing::Test,
                              public factor::modif::Factor {
protected:
  FactorCorrelationTest(bool corrOrAnti)
      : factor::modif::Factor(std::set<VariablePtr>{makeVariable(4, "A"),
                                                    makeVariable(4, "B"),
                                                    makeVariable(4, "C")},
                              corrOrAnti){};
};

class SimpleCorrelationTest : public FactorCorrelationTest {
public:
  SimpleCorrelationTest() : FactorCorrelationTest(true){};
};
TEST_F(SimpleCorrelationTest, simpleCorrelation) {
  std::size_t varSize = (*this->group->getVariables().begin())->size();
  EXPECT_EQ(this->values->size(), varSize);
  std::vector<std::size_t> vals(this->group->getVariables().size(), 0);
  for (auto it = this->values->begin(); it != this->values->end(); ++it) {
    test::compare(Combination(vals.data(), vals.size()), it->first);
    EXPECT_EQ(it->second, 1.f);
    for (std::size_t k = 0; k < vals.size(); ++k) {
      ++vals[k];
    }
  }
}

class SimpleAntiCorrelationTest : public FactorCorrelationTest {
public:
  SimpleAntiCorrelationTest() : FactorCorrelationTest(false){};
};
TEST_F(SimpleAntiCorrelationTest, simpleAntiCorrelation) {
  std::size_t varSize = (*this->group->getVariables().begin())->size();
  EXPECT_EQ(this->values->size(), this->group->size() - varSize);
  std::vector<std::size_t> vals(this->group->getVariables().size(), 0);
  for (auto it = this->values->begin(); it != this->values->end(); ++it) {
    EXPECT_EQ(it->second, 1.f);
  }
  for (std::size_t v = 0; v < varSize; ++v) {
    EXPECT_EQ(this->values->end(),
              this->values->find(Combination(vals.data(), vals.size())));
    for (std::size_t k = 0; k < vals.size(); ++k) {
      ++vals[k];
    }
  }
}

TEST(ConstFactor, mergeDistributions) {
  float val1 = 2.f, val2 = 0.5f;

  factor::modif::Factor distrAC(std::set<categoric::VariablePtr>(
      {makeVariable(2, "A"), makeVariable(2, "C")}));
  distrAC.setAllImagesRaw(val1);
  factor::modif::Factor distrBC(std::set<categoric::VariablePtr>(
      {makeVariable(2, "B"), makeVariable(2, "C")}));
  distrBC.setAllImagesRaw(val2);

  factor::modif::Factor distrABC(&distrAC, &distrBC);
  EXPECT_EQ(3, distrABC.getGroup().getVariables().size());
  {
    auto it = distrABC.getGroup().getVariables().begin();
    EXPECT_EQ(std::string("A"), (*it)->name());
    ++it;
    EXPECT_EQ(std::string("B"), (*it)->name());
    ++it;
    EXPECT_EQ(std::string("C"), (*it)->name());
  }

  std::size_t numberValues = 0;
  auto it = distrABC.getIterator();
  iterator::forEach(it, [&](const DistributionIterator &it) {
    EXPECT_EQ(it.getImage(), val1 * val2);
    ++numberValues;
  });
  EXPECT_EQ(numberValues, distrABC.getGroup().size());
}

TEST(ConstFactor, marginalization) {
  VariablePtr A = makeVariable(2, "A");
  VariablePtr B = makeVariable(3, "B");
  VariablePtr C = makeVariable(2, "C");
  VariablePtr D = makeVariable(3, "D");

  factor::modif::Factor distrABCD(
      std::set<categoric::VariablePtr>({A, B, C, D}));
  std::set<VariablePtr> groupBD = {B, D};

  std::vector<std::size_t> vals;

  Range rangeBD(groupBD);

  {
    iterator::forEach(rangeBD, [&vals, &distrABCD](const Range &rangeBD) {
      vals = std::vector<std::size_t>{1, rangeBD.get().data()[0], 1,
                                      rangeBD.get().data()[1]};
      distrABCD.setImageRaw(Combination(vals.data(), vals.size()), 1.f);
    });
  }

  {
    // marginalization A=0, C=1
    vals = {0, 1};
    factor::cnst::Factor distrBD(distrABCD,
                                 Combination(vals.data(), vals.size()), {A, C});
    rangeBD.reset();
    iterator::forEach(rangeBD, [&distrBD](const Range &rangeBD) {
      EXPECT_EQ(distrBD.find(rangeBD.get()), 0.f);
    });
  }

  {
    // marginalization A=1, C=1
    vals = {1, 1};
    factor::cnst::Factor distrBD(distrABCD,
                                 Combination(vals.data(), vals.size()), {A, C});
    rangeBD.reset();
    iterator::forEach(rangeBD, [&distrBD](const Range &rangeBD) {
      EXPECT_EQ(distrBD.find(rangeBD.get()), 1.f);
    });
  }
}

TEST(ConstFactor, importFromFile) {
  factor::cnst::Factor factor(
      std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")},
      std::string(TEST_FOLDER) + std::string("FactorDescription"));
}

// add test on factor merging

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
