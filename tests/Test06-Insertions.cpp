#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include <EasyFactorGraph/structure/FactorsManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>
using namespace EFG;
using namespace EFG::categoric;
using namespace EFG::distribution;
using namespace EFG::strct;
using namespace EFG::train;

namespace {
#define THROW_IF_TRUE(EXPRESSION, REASON)                                      \
  if (EXPRESSION) {                                                            \
    throw Error{REASON};                                                       \
  }

#define THROW_IF_FALSE(EXPRESSION, REASON) THROW_IF_TRUE(!EXPRESSION, REASON)

class Checker : virtual public ConnectionsManager {
public:
  void checkPostInsertion() {
    THROW_IF_TRUE(hasPropagationResult(),
                  "Expected to not have propagation result");
    const auto &state = getState();
    THROW_IF_FALSE(state.evidences.empty(), "Expected to have no evidences");

    THROW_IF_FALSE(
        state.nodes.size() == state.variables.size(),
        "number of nodes expected to be equal to number of variables");
    std::size_t hidden_nodes_numb = 0;
    for (const auto &cluster : state.clusters) {
      hidden_nodes_numb += cluster.nodes.size();
    }
    THROW_IF_FALSE(
        hidden_nodes_numb == state.nodes.size(),
        "number of hidden nodes expected to be equal to total number of nodes");
  };

  void checkVariables(const VariablesSet &vars) {
    const auto &nodes = getState().nodes;
    for (const auto &var : vars) {
      if (nodes.find(var) == nodes.end()) {
        throw Error{var->name(), " is a not found variable"};
      }
    }
  }

protected:
  Checker() = default;
};

class FactorsManagerTest : public Checker, public FactorsAdder {
public:
  FactorsManagerTest() = default;
};

bool have_same_values(const Distribution &a, const Distribution &b) {
  return (a.getVariables().getVariables() == b.getVariables().getVariables()) &&
         (a.getCombinationsMap() == b.getCombinationsMap());
}
} // namespace

TEST_CASE("const factors insertion", "[insertion][const]") {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");

  FactorsManagerTest model;

  SECTION("by copy") {
    distribution::Factor to_insert(Group{VariablesSoup{A, B}},
                                   USE_SIMPLE_CORRELATION_TAG);

    model.copyConstFactor(to_insert);
    model.checkPostInsertion();
    model.checkVariables(to_insert.getVariables().getVariablesSet());

    CHECK(model.getAllFactors().size() == 1);
    CHECK(model.getConstFactors().size() == 1);
    CHECK(have_same_values(**model.getAllFactors().begin(), to_insert));
    CHECK(have_same_values(**model.getConstFactors().begin(), to_insert));
  }

  SECTION("by sharing") {
    std::shared_ptr<Factor> to_insert = std::make_shared<Factor>(
        Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG);

    model.addConstFactor(to_insert);
    model.checkPostInsertion();
    model.checkVariables(to_insert->getVariables().getVariablesSet());

    CHECK(model.getAllFactors().size() == 1);
    CHECK(model.getConstFactors().size() == 1);

    CHECK(*model.getAllFactors().begin() == to_insert);
    CHECK(*model.getConstFactors().begin() == to_insert);
  }
}

namespace {
class FactorsTunableManagerTest : public Checker,
                                  public FactorsAdder,
                                  public FactorsTunableAdder {
public:
  FactorsTunableManagerTest() = default;

  virtual std::vector<float>
  getWeightsGradient(const TrainSet::Iterator &train_set_combinations) final {
    return {};
  }

  const std::vector<TunerPtr> &getTuners() const { return tuners; }

  const GraphState &accessState() const { return getState(); }
};
} // namespace

TEST_CASE("tunable factors insertion", "[insertion][tunable]") {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");

  FactorsTunableManagerTest model;

  SECTION("by copy") {
    distribution::FactorExponential to_insert(distribution::Factor{
        Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG});

    model.copyTunableFactor(to_insert);
    model.checkPostInsertion();
    model.checkVariables(to_insert.getVariables().getVariablesSet());

    CHECK(model.getAllFactors().size() == 1);
    CHECK(model.getConstFactors().size() == 0);
    CHECK(model.getTunableFactors().size() == 1);
    CHECK(model.getTuners().size() == 1);
    CHECK(have_same_values(**model.getAllFactors().begin(), to_insert));
    CHECK(have_same_values(**model.getTunableFactors().begin(), to_insert));
  }

  SECTION("by sharing") {
    std::shared_ptr<distribution::FactorExponential> to_insert =
        std::make_shared<distribution::FactorExponential>(distribution::Factor{
            Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG});

    model.addTunableFactor(to_insert);
    model.checkPostInsertion();
    model.checkVariables(to_insert->getVariables().getVariablesSet());

    CHECK(model.getAllFactors().size() == 1);
    CHECK(model.getConstFactors().size() == 0);
    CHECK(model.getTunableFactors().size() == 1);
    CHECK(model.getTuners().size() == 1);
    CHECK(*model.getAllFactors().begin() == to_insert);
    CHECK(*model.getTunableFactors().begin() == to_insert);
  }
}

TEST_CASE("tunable factors multiple insertions", "[insertion][tunable]") {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  auto C = make_variable(2, "C");
  auto D = make_variable(2, "D");

  FactorsTunableManagerTest model;

  distribution::FactorExponential factor_AB(distribution::Factor{
      Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG});
  distribution::Factor factor_BC(Group{VariablesSoup{B, C}},
                                 USE_SIMPLE_CORRELATION_TAG);
  distribution::FactorExponential factor_D(
      distribution::Factor{Group{VariablesSoup{D}}});

  model.copyTunableFactor(factor_AB);
  model.copyTunableFactor(factor_BC);
  model.copyTunableFactor(factor_D);
  model.checkVariables(VariablesSet{A, B, C, D});

  CHECK(model.getAllFactors().size() == 1);
  CHECK(model.getConstFactors().size() == 1);
  CHECK(model.getTunableFactors().size() == 2);
  CHECK(model.getTuners().size() == 2);

  const auto &state = model.accessState();
  CHECK(state.clusters.size() == 2);
  CHECK((state.clusters.front().nodes.size() == 3 ||
         state.clusters.back().nodes.size() == 3));
  CHECK((state.clusters.front().nodes.size() == 1 ||
         state.clusters.back().nodes.size() == 1));
}

TEST_CASE("check bad factor insertions are refused", "[insertion][bad]") {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");

  FactorsTunableManagerTest model;

  DistributionCnstPtr to_insert1 = std::make_shared<Factor>(
      Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG);
  model.addConstFactor(to_insert1);

  SECTION("factor connecting same variables") {
    DistributionCnstPtr to_insert2 = std::make_shared<Factor>(
        Group{VariablesSoup{A, B}}, USE_SIMPLE_CORRELATION_TAG);

    CHECK_THROWS_AS(model.addConstFactor(to_insert2), Error);
  }

  SECTION("factor referring to a bad variable") {
    auto A_bis = make_variable(2, "A");

    SECTION("bad unary factor") {
      DistributionCnstPtr to_insert2 =
          std::make_shared<Factor>(Group{VariablesSoup{A_bis}});

      CHECK_THROWS_AS(model.addConstFactor(to_insert2), Error);
    }

    SECTION("bad binary factor") {
      auto C = make_variable(2, "C");
      DistributionCnstPtr to_insert2 = std::make_shared<Factor>(
          Group{VariablesSoup{A_bis, C}}, USE_SIMPLE_CORRELATION_TAG);

      CHECK_THROWS_AS(model.addConstFactor(to_insert2), Error);
    }
  }
}
