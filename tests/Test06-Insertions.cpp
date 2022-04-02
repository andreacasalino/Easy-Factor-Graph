#include <gtest/gtest.h>

#include <EasyFactorGraph/structure/FactorsManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::strct;
using namespace EFG::train;

class Checker : virtual public ConnectionsManager {
protected:
  Checker() = default;

  void checkPostInsertion() {
    EXPECT_FALSE(hasPropagationResult());
    const auto &state = getState();
    EXPECT_TRUE(state.evidences.empty());
    EXPECT_EQ(state.nodes.size(), state.variables.size());
    std::size_t hidden_nodes_numb = 0;
    for (const auto &cluster : state.clusters) {
      hidden_nodes_numb += cluster.nodes.size();
    }
    EXPECT_EQ(hidden_nodes_numb, state.nodes.size());
  };

  void checkVariables(const VariablesSet &vars) {
    const auto &nodes = getState().nodes;
    for (const auto &var : vars) {
      if (nodes.find(var) == nodes.end()) {
        throw Error{var->name(), " is a not found variable"};
      }
    }
  }
};

class FactorsManagerTest : public ::testing::Test,
                           public Checker,
                           public FactorsAdder {
public:
  FactorsManagerTest() = default;
};

bool have_same_values(const Distribution &a, const Distribution &b) {
  return (a.getVariables().getVariables() == b.getVariables().getVariables()) &&
         (a.getCombinationsMap() == b.getCombinationsMap());
}

TEST_F(FactorsManagerTest, insertFactorCopy) {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  distribution::Factor to_insert(Group{VariablesSoup{A, B}},
                                 USE_SIMPLE_CORRELATION_TAG);

  copyConstFactor(to_insert);
  checkPostInsertion();
  checkVariables(to_insert.getVariables().getVariablesSet());

  EXPECT_EQ(getAllFactors().size(), 1);
  EXPECT_EQ(getConstFactors().size(), 1);
  EXPECT_TRUE(have_same_values(**getConstFactors().begin(), to_insert));
}

TEST_F(FactorsManagerTest, insertFactorShared) {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  std::shared_ptr<Factor> to_insert = std::make_shared<Factor>(
      Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG);

  addConstFactor(to_insert);
  checkPostInsertion();
  checkVariables(to_insert->getVariables().getVariablesSet());

  EXPECT_EQ(getAllFactors().size(), 1);
  EXPECT_EQ(getConstFactors().size(), 1);

  EXPECT_EQ(*getAllFactors().begin(), to_insert);
  EXPECT_EQ(*getConstFactors().begin(), to_insert);
}

class FactorsTunableManagerTest : public ::testing::Test,
                                  public Checker,
                                  public FactorsAdder,
                                  public FactorsTunableAdder {
public:
  FactorsTunableManagerTest() = default;

  virtual std::vector<float>
  getWeightsGradient(const TrainSet::Iterator &train_set_combinations) final {
    return {};
  }
};

TEST_F(FactorsTunableManagerTest, insertTunableFactorCopy) {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  distribution::FactorExponential to_insert(distribution::Factor{
      Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG});

  copyTunableFactor(to_insert);
  checkPostInsertion();
  checkVariables(to_insert.getVariables().getVariablesSet());

  EXPECT_EQ(getAllFactors().size(), 1);
  EXPECT_EQ(getConstFactors().size(), 0);
  EXPECT_EQ(getTunableFactors().size(), 1);
  EXPECT_EQ(tuners.size(), 1);
  EXPECT_TRUE(have_same_values(**getTunableFactors().begin(), to_insert));
}

TEST_F(FactorsTunableManagerTest, insertTunableFactorShared) {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  std::shared_ptr<distribution::FactorExponential> to_insert =
      std::make_shared<distribution::FactorExponential>(distribution::Factor{
          Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG});

  addTunableFactor(to_insert);
  checkPostInsertion();
  checkVariables(to_insert->getVariables().getVariablesSet());

  EXPECT_EQ(getAllFactors().size(), 1);
  EXPECT_EQ(getConstFactors().size(), 0);
  EXPECT_EQ(getTunableFactors().size(), 1);
  EXPECT_EQ(tuners.size(), 1);
  EXPECT_EQ(*getAllFactors().begin(), to_insert);
  EXPECT_EQ(*getTunableFactors().begin(), to_insert);
}

TEST_F(FactorsTunableManagerTest, insertTunableFactorCopy_multiple) {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  auto C = make_variable(2, "C");
  auto D = make_variable(2, "D");
  distribution::FactorExponential factor_AB(distribution::Factor{
      Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG});
  distribution::Factor factor_BC(Group{VariablesSoup{B, C}},
                                 USE_SIMPLE_CORRELATION_TAG);
  distribution::FactorExponential factor_D(
      distribution::Factor{Group{VariablesSoup{D}}});

  copyTunableFactor(factor_AB);
  copyTunableFactor(factor_BC);
  copyTunableFactor(factor_D);
  checkVariables(VariablesSet{A, B, C, D});

  EXPECT_EQ(getAllFactors().size(), 1);
  EXPECT_EQ(getConstFactors().size(), 1);
  EXPECT_EQ(getTunableFactors().size(), 2);
  EXPECT_EQ(tuners.size(), 2);

  const auto &state = getState();
  EXPECT_EQ(state.clusters.size(), 2);
  EXPECT_TRUE(state.clusters.front().nodes.size() == 3 ||
              state.clusters.back().nodes.size() == 3);
  EXPECT_TRUE(state.clusters.front().nodes.size() == 1 ||
              state.clusters.back().nodes.size() == 1);
}

TEST_F(FactorsManagerTest, refuseInsertionOverSameVariables) {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  DistributionCnstPtr to_insert1 = std::make_shared<Factor>(
      Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG);
  DistributionCnstPtr to_insert2 = std::make_shared<Factor>(
      Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG);

  addConstFactor(to_insert1);
  ASSERT_THROW(addConstFactor(to_insert2), Error);
}

TEST_F(FactorsManagerTest, refuseInsertionBadVariable) {
  auto A1 = make_variable(2, "A");
  auto A2 = make_variable(2, "A");
  auto B = make_variable(2, "B");
  DistributionCnstPtr to_insert1 = std::make_shared<Factor>(
      Group{VariablesSoup{A1, B}}, USE_SIMPLE_CORRELATION_TAG);
  DistributionCnstPtr to_insert2 =
      std::make_shared<Factor>(Group{VariablesSoup{A2}});

  addConstFactor(to_insert1);
  ASSERT_THROW(addConstFactor(to_insert2), Error);
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
