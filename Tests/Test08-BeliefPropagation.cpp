#include <gtest/gtest.h>
#include <ModelTest.h>
#include <model/Graph.h>
#include <distribution/factor/const/Indicator.h>
#include <math.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::model;

void compare(const std::vector<float>& probTheory, const std::vector<float>& probComputed, const float threshold = 0.01f) {
  EXPECT_EQ(probTheory.size(), probComputed.size());
 
  auto probNormalized = probTheory;
  float Sum = 0.f;
  for(auto itA =probTheory.begin(); itA != probTheory.end(); ++itA) {
    Sum += *itA;
  }
  std::vector<float>::iterator itA;
  for(itA =probNormalized.begin(); itA != probNormalized.end(); ++itA) {
    *itA /= Sum;
  }
  auto itB = probComputed.begin();
  for(itA =probNormalized.begin(); itA!=probNormalized.end(); ++itA) {
    EXPECT_LE(fabs(*itA - *itB) , threshold);
    ++itB;
  }
}

class MessageChecker
    : virtual public Graph {
protected:
    // check all messages were computed after propagation
    void checkMessages() {
        auto checkCluster = [](const std::set<strct::Node*>& cluster) {
            for (auto c = cluster.begin(); c != cluster.end(); ++c) {
                for (auto m = (*c)->activeConnections.begin(); m != (*c)->activeConnections.end(); ++m) {
                    EXPECT_TRUE(nullptr != m->second.message2This);
                }
            }
        };
        for (auto it = this->hidden.clusters.begin(); it != this->hidden.clusters.end(); ++it) {
            checkCluster(*it);
        }
    };
};

class SimplePolyTreeModel 
    : public MessageChecker
    , public test::ModelTest<Graph> {
public:
  SimplePolyTreeModel() =  default;
protected:
  std::string getName() const final { return "graph_1.xml"; };
};
TEST_F(SimplePolyTreeModel, simplePolyTree) {
	float a = expf(1.f), b = expf(2.f), g = expf(1.f), e = expf(1.5f);

  // E=1
  this->resetEvidences({ {"E", 1} });
  compare({ (a * (g + e) + (1 + g * e)), ((g + e) + a * (1 + g * e)) }, this->getMarginalDistribution("A"));
  this->checkMessages();
  compare({ (g + e) ,(1 + g * e) }, this->getMarginalDistribution("B"));
  compare({ (b * (g + e) + (1 + g * e)), ((g + e) + b * (1 + g * e)) }, this->getMarginalDistribution("C"));
  compare({ 1.f, e }, this->getMarginalDistribution("D"));

  // D=1
  this->resetEvidences({ {"D", 1} });
  compare({ a + g, 1.f + a * g }, this->getMarginalDistribution("A"));
  compare({ 1.f, g }, this->getMarginalDistribution("B"));
  compare({ b + g, 1.f + b * g }, this->getMarginalDistribution("C"));
  compare({ 1.f, e }, this->getMarginalDistribution("E"));
}

class ComplexPolyTreeModel
    : public MessageChecker
    , public test::ModelTest<Graph> {
public:
  ComplexPolyTreeModel() =  default;
protected:
  std::string getName() const final { return "graph_2.xml"; };
};
TEST_F(ComplexPolyTreeModel, complexPolyTree) {
  this->resetEvidences({{"v1", 1}, {"v2", 1}, { "v3", 1 }});

  {
    auto prob = this->getMarginalDistribution("v10");
    EXPECT_GT(prob[1], prob[0]);
  }

  {
    auto prob = this->getMarginalDistribution("v11");
    EXPECT_GT(prob[1], prob[0]);
  }

  {
    auto prob = this->getMarginalDistribution("v12");
    EXPECT_GT(prob[1], prob[0]);
  }

  this->checkMessages();
}

TEST_F(ComplexPolyTreeModel, complexPolyTreeThreadPool) {
  this->resetEvidences({{"v1", 1}, {"v2", 1}, { "v3", 1 }});
  this->setThreadPoolSize(3);

  {
    auto prob = this->getMarginalDistribution("v10");
    EXPECT_GT(prob[1], prob[0]);
  }

  {
    auto prob = this->getMarginalDistribution("v11");
    EXPECT_GT(prob[1], prob[0]);
  }

  {
    auto prob = this->getMarginalDistribution("v12");
    EXPECT_GT(prob[1], prob[0]);
  }

  this->checkMessages();
}

class SimpleLoopyModel
    : public MessageChecker
    , public test::ModelTest<Graph>{
public:
  SimpleLoopyModel() =  default;
protected:
  std::string getName() const final { return "graph_3.xml"; };
};
TEST_F(SimpleLoopyModel, simpleLoopy) {
  float M = expf(1.f);
  float M_alfa = powf(M, 3) + M + 2.f*powf(M, 2);
  float M_beta = powf(M, 4) + 2.f*M + powf(M, 2);

  // E=1
  this->resetEvidences({ {"E", 1} });
  compare({ 3.f * M + powf(M,3), powf(M,4) + 3.f * powf(M,2) }, this->getMarginalDistribution("D"), 0.045f);
  this->checkMessages();
  compare({ M_alfa, M_beta }, this->getMarginalDistribution("C"), 0.045f);
  compare({ M_alfa, M_beta }, this->getMarginalDistribution("B"), 0.045f);
  compare({ M * M_alfa + M_beta, M_alfa + M * M_beta }, this->getMarginalDistribution("A"), 0.045f);
}

class ComplexLoopyModel
    : public MessageChecker
    , public test::ModelTest<Graph> {
public:
  ComplexLoopyModel() =  default;
protected:
  std::string getName() const final { return "graph_4.xml"; };
};
TEST_F(ComplexLoopyModel, complexLoopy) {
  this->resetEvidences({{"v1", 1}});

  {
    auto prob = this->getMarginalDistribution("v8");
    EXPECT_GT(prob[1], prob[0]);
  }

  this->checkMessages();
}

TEST_F(ComplexLoopyModel, complexLoopyThreadPool) {
  this->resetEvidences({{"v1", 1}});
  this->setThreadPoolSize(3);

  {
    auto prob = this->getMarginalDistribution("v8");
    EXPECT_GT(prob[1], prob[0]);
  }

  this->checkMessages();
}

void fillChain(model::Graph& model, const float wXY, const float wYY) {
  std::vector<VariablePtr> Y = {makeVariable(2, "Y0"), makeVariable(2, "Y1"),makeVariable(2, "Y2"),makeVariable(2, "Y3")};
  std::vector<VariablePtr> X = {makeVariable(2, "X0"), makeVariable(2, "X1"),makeVariable(2, "X2"),makeVariable(2, "X3")};

  auto connect = [&model](const VariablePtr& a, const VariablePtr& b, const float w){
    model.insertCopy(factor::cnst::FactorExponential(factor::cnst::Factor(std::set<VariablePtr>{a, b}, true) , w));
  };

  connect(X[0], Y[0], wXY);
  connect(X[1], Y[1], wXY);
  connect(X[2], Y[2], wXY);
  connect(X[3], Y[3], wXY);

  connect(Y[0], Y[1], wYY);
  connect(Y[1], Y[2], wYY);
  connect(Y[2], Y[3], wYY);

  model.insertCopy(factor::cnst::FactorExponential(factor::cnst::IndicatorFactor(Y[0], 1), wYY));

  model.resetEvidences({{"X0" , 0},{"X1" , 1},{"X2" , 0},{"X3" , 1}});
};

TEST(MAPTest, map1) {
  model::Graph model;
  fillChain(model, 0.1f, 1.f);
  std::vector<std::size_t> mapExpected = {1,1,1,1}; 
  EXPECT_EQ(mapExpected, model.getHiddenSetMAP());
}

TEST(MAPTest, map2) {
  model::Graph model;
  fillChain(model, 1.0f, 0.1f);
  std::vector<std::size_t> mapExpected = {0,1,0,1}; 
  EXPECT_EQ(mapExpected, model.getHiddenSetMAP());
}

TEST(SubGroupDistribution, subGroupDistribution) {
  VariablePtr A = makeVariable(2, "A");
  VariablePtr B = makeVariable(2, "B");
  VariablePtr C = makeVariable(2, "C");
  VariablePtr D = makeVariable(2, "D");
  float alfa = 0.5f, beta = 1.5f;        
  //build the chain
  model::Graph graph;
  graph.insert(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor({ A, B }, true), alfa));
  graph.insert(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor({ B, C }, true), beta));
  graph.insert(std::make_shared<factor::modif::FactorExponential>(factor::cnst::Factor({ C, D }, true), 1.f));
  
  // joint distribution of A B C
  compare({expf(alfa) * expf(beta),
          expf(alfa),
          1.f,
          expf(beta),
          expf(beta),
          1.f,
          expf(alfa),
          expf(alfa) * expf(beta)} , graph.getJointMarginalDistribution({"A", "B", "C"}).getProbabilities());

  // joint distribution of A B
  compare({ expf(alfa), 1.f, 1.f, expf(alfa) }, graph.getJointMarginalDistribution({"A", "B"}).getProbabilities());
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}