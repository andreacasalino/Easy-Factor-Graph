#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>

#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/structure/FactorsManager.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::strct;

class EvidenceTest : public ::testing::Test,
                     public EvidenceSetter,
                     public EvidenceRemover,
                     protected FactorsAdder {
public:
  EvidenceTest() = default;

protected:
  VariablesSoup uVars;
  VariablesSoup mVars;
  VariablesSoup lVars;

  void SetUp() override {
    auto connect = [this](const VariablePtr &a, const VariablePtr &b) {
      DistributionCnstPtr factor = std::make_shared<Factor>(
          Group{VariablesSoup{a, b}}, USE_SIMPLE_CORRELATION_TAG);
      addConstFactor(factor);
    };

    auto createVariable = [](const std::string &name, std::size_t id) {
      std::stringstream s;
      s << name << id;
      return make_variable(2, s.str());
    };

    this->uVars.reserve(3);
    this->mVars.reserve(4);
    this->lVars.reserve(3);

    this->uVars.push_back(createVariable("A", 0));
    this->mVars.push_back(createVariable("M", 0));
    this->lVars.push_back(createVariable("L", 0));
    connect(this->uVars.back(), this->mVars.back());
    connect(this->lVars.back(), this->mVars.back());

    for (std::size_t k = 1; k < 3; ++k) {
      std::size_t s = this->mVars.size();
      this->uVars.push_back(createVariable("A", k));
      this->mVars.push_back(createVariable("M", k));
      this->lVars.push_back(createVariable("L", k));
      connect(this->uVars.back(), this->mVars.back());
      connect(this->lVars.back(), this->mVars.back());
      connect(this->uVars[s - 1], this->mVars.back());
      connect(this->lVars[s - 1], this->mVars.back());
    }
  }

  bool clusterExists(const VariablesSet &vars) {
    std::set<strct::Node *> nodes;
    {
      for (const auto &var : vars) {
        auto nodes_it = getState_().nodes.find(var);
        if (nodes_it == getState_().nodes.end()) {
          throw Error{nodes_it->first->name(),
                      " is not part of the hidden clusters"};
        }
        nodes.emplace(&nodes_it->second);
      }
    }
    return std::find_if(getState().clusters.begin(), getState().clusters.end(),
                        [&nodes](const HiddenCluster &cluster) {
                          return cluster.nodes == nodes;
                        }) != getState().clusters.end();
  };
};

TEST_F(EvidenceTest, evidenceAddition) {
  setEvidence(mVars[1], 0);
  clusterExists(VariablesSet{uVars[0], mVars[0], lVars[0]});
  clusterExists(VariablesSet{uVars[2], mVars[2], lVars[2], uVars[1], lVars[1]});
  {
    Evidences expected;
    expected.emplace(mVars[1], 0);
    EXPECT_EQ(getEvidences(), expected);
  }

  setEvidence(mVars[2], 1);
  clusterExists(VariablesSet{uVars[0], mVars[0], lVars[0]});
  clusterExists(VariablesSet{uVars[2], mVars[2], lVars[2]});
  clusterExists(VariablesSet{uVars[1]});
  clusterExists(VariablesSet{lVars[1]});
  {
    Evidences expected;
    expected.emplace(mVars[1], 0);
    expected.emplace(mVars[2], 1);
    EXPECT_EQ(getEvidences(), expected);
  }
}

TEST_F(EvidenceTest, evidenceSingleReset) {
  setEvidence(mVars[1], 0);
  setEvidence(mVars[2], 0);

  removeEvidence(mVars[2]);
  clusterExists(VariablesSet{uVars[0], mVars[0], lVars[0]});
  clusterExists(VariablesSet{uVars[2], mVars[2], lVars[2], uVars[1], lVars[1]});
  {
    Evidences expected;
    expected.emplace(mVars[1], 0);
    EXPECT_EQ(getEvidences(), expected);
  }

  removeEvidence(mVars[1]);
  clusterExists(
      VariablesSet{getAllVariables().begin(), getAllVariables().end()});
  EXPECT_EQ(getEvidences(), Evidences{});
}

TEST_F(EvidenceTest, evidenceTotalReset) {
  setEvidence(mVars[1], 0);
  setEvidence(mVars[2], 0);
  removeEvidences();
  EXPECT_EQ(getEvidences(), Evidences{});
  ASSERT_THROW(removeEvidence(mVars[2]), Error);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
