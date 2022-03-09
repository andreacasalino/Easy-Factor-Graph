#include <gtest/gtest.h>
#include <model/Graph.h>
#include <sstream>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;

class GraphTest 
: public ::testing::Test
, public model::Graph { 
public:
  GraphTest() = default;

protected: 
  void SetUp() override {
    auto connect = [this](const VariablePtr& a, const VariablePtr& b){
      this->insert(std::make_shared<factor::cnst::Factor>(std::set<VariablePtr>{a, b} , true));
    };

    auto createVariable = [](const std::string& name, std::size_t id){
      std::stringstream s;
      s << name << id;
      return makeVariable(2, s.str());
    };

    this->uVars.reserve(3);
    this->mVars.reserve(4);
    this->lVars.reserve(3);

    this->uVars.push_back(createVariable("A", 0));
    this->mVars.push_back(createVariable("M", 0));
    this->lVars.push_back(createVariable("L", 0));
    connect(this->uVars.back(), this->mVars.back());
    connect(this->lVars.back(), this->mVars.back());

    for(std::size_t k=1; k<3; ++k) {
      std::size_t s = this->mVars.size();
      this->uVars.push_back(createVariable("A", k));
      this->mVars.push_back(createVariable("M", k));
      this->lVars.push_back(createVariable("L", k));
      connect(this->uVars.back(), this->mVars.back());
      connect(this->lVars.back(), this->mVars.back());
      connect(this->uVars[s-1], this->mVars.back());
      connect(this->lVars[s-1], this->mVars.back());
    }
  }

  bool clusterExists(const std::set<VariablePtr>& vars) {
    std::set<strct::Node*> cluster;
    for (auto it = vars.begin(); it!=vars.end(); ++it) {
      cluster.emplace(&this->nodes.find(*it)->second);
    }
    for(auto itCl = this->hidden.clusters.begin(); itCl != this->hidden.clusters.end(); ++itCl) {
      if(*itCl == cluster) {
        return true;
      }
    }
    return false;
  };

  void compare(const std::map<std::string, std::size_t>& ob) {
    EXPECT_EQ(ob.size(), this->evidences.size());
    auto itob = ob.begin();
    for(auto it = this->evidences.begin(); it!=this->evidences.end(); ++it) {
      EXPECT_EQ(it->first->name(), itob->first);
      EXPECT_EQ(it->second, itob->second);
      ++itob;
    }
  };

  std::vector<VariablePtr> uVars;
  std::vector<VariablePtr> mVars;
  std::vector<VariablePtr> lVars;
};

TEST_F(GraphTest, resetEvidences1) {
  std::map<std::string, std::size_t> ob = {{"M0" , 0}};

  this->resetEvidences(ob);
  this->compare(ob);
  EXPECT_EQ(this->hidden.clusters.size(), 1);
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[0], this->lVars[0],
                                                        this->uVars[1], this->mVars[1], this->lVars[1],
                                                        this->uVars[2], this->mVars[2], this->lVars[2]}));
}

TEST_F(GraphTest, resetEvidences2) {
  std::map<std::string, std::size_t> ob = {{"M1" , 0}};

  this->resetEvidences(ob);
  this->compare(ob);
  EXPECT_EQ(this->hidden.clusters.size(), 2);
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[0], this->mVars[0], this->lVars[0]}));
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[2], this->mVars[2], this->lVars[2], this->uVars[1], this->lVars[1]}));
}

TEST_F(GraphTest, resetEvidences3) {
  std::map<std::string, std::size_t> ob = {{"M0" , 1}, {"M1" , 0}};

  this->resetEvidences(ob);
  this->compare(ob);
  EXPECT_EQ(this->hidden.clusters.size(), 3);
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[2], this->mVars[2], this->lVars[2], this->uVars[1], this->lVars[1]}));
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[0]}));
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->lVars[0]}));
}

TEST_F(GraphTest, addEvidence) {
  this->resetEvidences({{"M0" , 0}});
  this->addEvidence("M1" , 1);

  this->compare({{"M0" , 0}, {"M1" , 1}});
  EXPECT_EQ(this->hidden.clusters.size(), 3);
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[2], this->mVars[2], this->lVars[2], this->uVars[1], this->lVars[1]}));
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[0]}));
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->lVars[0]}));

  ASSERT_THROW(this->addEvidence("M1" , 1), Error);
}

TEST_F(GraphTest, setEvidences) {
  this->resetEvidences({{"M0" , 1}, {"M1" , 0}});

  this->setEvidences({1,1});
  this->compare({{"M0" , 1}, {"M1" , 1}});
  EXPECT_EQ(this->hidden.clusters.size(), 3);
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[2], this->mVars[2], this->lVars[2], this->uVars[1], this->lVars[1]}));
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->uVars[0]}));
  EXPECT_TRUE(this->clusterExists(std::set<VariablePtr>{this->lVars[0]}));

  ASSERT_THROW(this->setEvidences({0}), Error);
  ASSERT_THROW(this->setEvidences({10,10}), Error);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
