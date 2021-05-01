#include <gtest/gtest.h>
#include <model/Graph.h>
#include <model/RandomField.h>
#include <distribution/factor/modifiable/Factor.h>
#include <DistributionCompare.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;

template<typename Model>
class ModelTest 
: public ::testing::Test
, public Model { 
public: 
    ModelTest() = default;

protected:
    void checkPostInsertion() {
        EXPECT_EQ(nullptr, this->lastPropagation);
        EXPECT_EQ(this->hidden.clusters.size(), 1);
        std::set<EFG::nodes::Node*> clusterExpected = std::set<EFG::nodes::Node*>{&this->nodes.begin()->second, &this->nodes.rbegin()->second};
        EXPECT_EQ(this->hidden.clusters.front(), clusterExpected);
    };
};

template<typename T>
bool containsElementPtr(const std::set<std::shared_ptr<T>>& group, const std::shared_ptr<T>& element) {
    auto it = group.find(element);
    if(it == group.end()) return false;
    return (it->get() == element.get());
};

template<typename T>
bool containsElementPtrs(const std::set<std::shared_ptr<T>>& group, const std::set<std::shared_ptr<T>>& elements) {
    for(auto it = elements.begin(); it!=elements.end(); ++it) {
        if(!containsElementPtr(group, *it)) {
            return false;
        }
    }
    return true;
};

typedef ModelTest<Graph> GraphTest;

TEST_F(GraphTest, insertFactorCopy) {
    factor::cnst::Factor factor(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , true);    
    this->insertCopy(factor);
    
    EXPECT_TRUE(containsElementPtrs(this->getVariables(), factor.getGroup().getVariables()));
    EXPECT_EQ(this->factors.size(), 1);
    EXPECT_EQ(this->factorsExp.size(), 0);
    test::compare(**this->factors.begin(), factor);
    this->checkPostInsertion();
}

TEST_F(GraphTest, insertFactorShared) {
    std::shared_ptr<factor::cnst::Factor> factor = std::make_shared<factor::cnst::Factor>(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , true);
    this->insert(factor);
    
    EXPECT_TRUE(containsElementPtrs(this->getVariables(), factor->getGroup().getVariables()));
    EXPECT_EQ(this->factors.size(), 1);
    EXPECT_EQ(this->factorsExp.size(), 0);
    EXPECT_EQ(factor.get(), this->factors.begin()->get());
    this->checkPostInsertion();
}

typedef ModelTest<RandomField> RandomFieldTest;

TEST_F(RandomFieldTest, insertFactorExpCopy) {
    float w = 2.f;
    factor::cnst::FactorExponential factor(factor::cnst::Factor(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , true), w);    
    this->insertCopy(factor);
    
    EXPECT_TRUE(containsElementPtrs(this->getVariables(), factor.getGroup().getVariables()));
    EXPECT_EQ(this->factors.size(), 0);
    EXPECT_EQ(this->factorsExp.size(), 1);
    EXPECT_EQ(this->factorsTunable.size(), 0);
    test::compare(**this->factorsExp.begin(), factor);
    this->checkPostInsertion();
}

TEST_F(RandomFieldTest, insertFactorExpShared) {
    float w = 2.f;
    std::shared_ptr<factor::cnst::FactorExponential> factor = std::make_shared<factor::cnst::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , true), w);    
    this->insert(factor);
    
    EXPECT_TRUE(containsElementPtrs(this->getVariables(), factor->getGroup().getVariables()));
    EXPECT_EQ(this->factors.size(), 0);
    EXPECT_EQ(this->factorsExp.size(), 1);
    EXPECT_EQ(this->factorsTunable.size(), 0);
    EXPECT_EQ(factor.get(), this->factorsExp.begin()->get());
    this->checkPostInsertion();
}

TEST_F(RandomFieldTest, insertTunableCopy) {
    // first insertion
    float w = 2.f;
    factor::modif::FactorExponential factor(factor::cnst::Factor(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , true), w);    
    this->insertTunableCopy(factor);
    
    EXPECT_TRUE(containsElementPtrs(this->getVariables(), factor.getGroup().getVariables()));
    EXPECT_EQ(this->factors.size(), 0);
    EXPECT_EQ(this->factorsExp.size(), 0);
    EXPECT_EQ(this->factorsTunable.size(), 1);
    EXPECT_EQ(this->factorsTunable.begin()->second, 0);
    // EXPECT_EQ(*this->factorsTunable.begin()->first, factor);
    EXPECT_EQ(this->numberOfClusters, 1);
    this->checkPostInsertion();

    // second insertion
    factor::modif::FactorExponential factor2(factor::cnst::Factor(std::set<VariablePtr>{makeVariable(2, "B"), makeVariable(2, "C")} , true), w);    
    this->insertTunableCopy(factor2);
    EXPECT_EQ(this->factorsTunable.size(), 2);
    {
        auto itTunab = this->findSharingFactor(factor2.getGroup().getVariables());
        EXPECT_EQ(itTunab->second, 1);
    }
    EXPECT_EQ(this->numberOfClusters, 2);

    // third insertion
    factor::modif::FactorExponential factor3(factor::cnst::Factor(std::set<VariablePtr>{makeVariable(2, "C"), makeVariable(2, "D")} , true), w + 1.f);    
    this->insertTunableCopy(factor3, factor.getGroup().getVariables());
    EXPECT_EQ(this->factorsTunable.size(), 3);
    {
        auto itTunab = this->findSharingFactor(factor3.getGroup().getVariables());
        EXPECT_EQ(itTunab->first->getWeight(), w);
        EXPECT_EQ(itTunab->second, 0);
    }
    EXPECT_EQ(this->numberOfClusters, 2);
}

TEST_F(RandomFieldTest, insertTunableShared) {
    // first insertion
    float w = 2.f;
    std::shared_ptr<factor::modif::FactorExponential> factor = std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , true), w);       
    this->insertTunable(factor);
    
    EXPECT_TRUE(containsElementPtrs(this->getVariables(), factor->getGroup().getVariables()));
    EXPECT_EQ(this->factors.size(), 0);
    EXPECT_EQ(this->factorsExp.size(), 0);
    EXPECT_EQ(this->factorsTunable.size(), 1);
    EXPECT_EQ(this->factorsTunable.begin()->second, 0);
    EXPECT_EQ(factor.get(), this->factorsTunable.begin()->first.get());
    EXPECT_EQ(this->numberOfClusters, 1);
    this->checkPostInsertion();

    // second insertion
    std::shared_ptr<factor::modif::FactorExponential> factor2 = std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{this->findVariable("B"), makeVariable(2, "C")} , true), w);       
    this->insertTunable(factor2);
    EXPECT_EQ(this->factorsTunable.size(), 2);
    {
        auto itTunab = this->findSharingFactor(factor2->getGroup().getVariables());
        EXPECT_EQ(itTunab->second, 1);
    }
    EXPECT_EQ(this->numberOfClusters, 2);

    // third insertion
    std::shared_ptr<factor::modif::FactorExponential> factor3 = std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor(std::set<VariablePtr>{this->findVariable("C"), makeVariable(2, "D")} , true), w+1.f);       
    this->insertTunable(factor3, factor->getGroup().getVariables());
    EXPECT_EQ(this->factorsTunable.size(), 3);
    {
        auto itTunab = this->findSharingFactor(factor3->getGroup().getVariables());
        EXPECT_EQ(itTunab->first->getWeight(), w);
        EXPECT_EQ(itTunab->second, 0);
    }
    EXPECT_EQ(this->numberOfClusters, 2);
}

TEST_F(GraphTest, refuseInsertionSameVariables) {
    factor::cnst::Factor factor1 = factor::cnst::Factor(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , true);    
    factor::cnst::Factor factor2 = factor::cnst::Factor(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , false);    
    this->insertCopy(factor1);
    ASSERT_THROW(this->insertCopy(factor2), Error);
}

TEST_F(GraphTest, refuseInsertionInexistentVariable) {
    std::shared_ptr<factor::cnst::Factor> factor1 = std::make_shared<factor::cnst::Factor>(std::set<VariablePtr>{makeVariable(2, "A"), makeVariable(2, "B")} , true);    
    this->insert(factor1);

    std::shared_ptr<factor::cnst::Factor> factor2 = std::make_shared<factor::cnst::Factor>(std::set<VariablePtr>{makeVariable(2, "B"), makeVariable(2, "C")} , false);    
    ASSERT_THROW(this->insert(factor2), Error);

    std::shared_ptr<factor::cnst::Factor> factor3 = std::make_shared<factor::modif::Factor>(std::set<VariablePtr>{makeVariable(2, "B")});    
    ASSERT_THROW(this->insert(factor3), Error);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
