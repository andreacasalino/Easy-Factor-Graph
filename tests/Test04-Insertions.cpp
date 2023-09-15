#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>

#include "Utils.h"
#include <EasyFactorGraph/structure/FactorsConstManager.h>
#include <EasyFactorGraph/trainable/FactorsTunableManager.h>

namespace EFG::test {
using namespace categoric;
using namespace factor;
using namespace strct;
using namespace train;
using namespace test;

namespace {
class Checker : virtual public FactorsAware {
public:
  void checkPostInsertion() {
    if (hasPropagationResult()) {
      throw Error{"Expected to not have propagation result"};
    }

    const auto &state = this->state();

    if (!state.evidences.empty()) {
      throw Error{"Expected to have no evidences"};
    }

    if (state.nodes.size() != state.variables.size()) {
      throw Error{
          "number of nodes expected to be equal to number of variables"};
    }

    std::size_t hidden_nodes_numb = 0;
    for (const auto &cluster : state.clusters) {
      hidden_nodes_numb += cluster.nodes.size();
    }

    if (hidden_nodes_numb != state.nodes.size()) {
      throw Error{"number of hidden nodes expected to be equal to total number "
                  "of nodes"};
    }
  };

  void checkVariables(const VariablesSet &vars) {
    VariablesSet vars_this;
    for (const auto &node : state().nodes) {
      vars_this.emplace(node.first);
    }
    if (vars_this != vars) {
      throw Error{"Variables mismatch"};
    }
  }

protected:
  Checker() = default;
};

class FactorsManagerTest : public Checker, public FactorsConstInserter {
public:
  FactorsManagerTest() = default;
};
} // namespace

TEST_CASE("const factors insertion", "[insertion]") {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");

  FactorsManagerTest model;

  SECTION("by copy") {
    auto to_insert = make_corr_factor(A, B);

    model.copyConstFactor(to_insert);
    model.checkPostInsertion();
    model.checkVariables(to_insert.function().vars().getVariablesSet());

    CHECK(model.getAllFactors().size() == 1);
    CHECK(model.getConstFactors().size() == 1);

    CHECK(test::almost_equal_fnct(
        model.getAllFactors().begin()->get()->function(),
        to_insert.function()));
    CHECK(test::almost_equal_fnct(
        model.getConstFactors().begin()->get()->function(),
        to_insert.function()));
  }

  SECTION("by sharing") {
    auto to_insert = make_corr_factor_ptr(A, B);

    model.addConstFactor(to_insert);
    model.checkPostInsertion();
    model.checkVariables(to_insert->function().vars().getVariablesSet());

    CHECK(model.getAllFactors().size() == 1);
    CHECK(model.getConstFactors().size() == 1);

    CHECK(*model.getAllFactors().begin() == to_insert);
    CHECK(*model.getConstFactors().begin() == to_insert);
  }
}

namespace {
class FactorsTunableManagerTest : public Checker,
                                  public FactorsConstInserter,
                                  public FactorsTunableInserter {
public:
  FactorsTunableManagerTest() = default;

  virtual std::vector<float>
  getWeightsGradient_(const TrainSet::Iterator &train_set_combinations) final {
    return {};
  }

  const Tuners &getTuners() const { return tuners; }

  using StateAware::state;
};
} // namespace

TEST_CASE("tunable factors insertion", "[insertion]") {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");

  FactorsTunableManagerTest model;

  SECTION("by copy") {
    auto to_insert = make_corr_expfactor(A, B, 1.f);

    model.copyTunableFactor(to_insert);
    model.checkPostInsertion();
    model.checkVariables(to_insert.function().vars().getVariablesSet());

    CHECK(model.getAllFactors().size() == 1);
    CHECK(model.getConstFactors().size() == 0);
    CHECK(model.getTunableFactors().size() == 1);
    CHECK(model.getTuners().size() == 1);
    CHECK(test::almost_equal_fnct(
        model.getAllFactors().begin()->get()->function(),
        to_insert.function()));
    CHECK(test::almost_equal_fnct(
        model.getTunableFactors().begin()->get()->function(),
        to_insert.function()));
  }

  SECTION("by sharing") {
    auto to_insert = make_corr_expfactor_ptr(A, B, 1.f);

    model.addTunableFactor(to_insert);
    model.checkPostInsertion();
    model.checkVariables(to_insert->function().vars().getVariablesSet());

    CHECK(model.getAllFactors().size() == 1);
    CHECK(model.getConstFactors().size() == 0);
    CHECK(model.getTunableFactors().size() == 1);
    CHECK(model.getTuners().size() == 1);
    CHECK(*model.getAllFactors().begin() == to_insert);
    CHECK(*model.getTunableFactors().begin() == to_insert);

    SECTION("add factor sharing the weight") {
      auto C = make_variable(2, "C");
      auto to_insert2 = make_corr_expfactor_ptr(A, C, 1.f);

      model.addTunableFactor(to_insert2,
                             to_insert->function().vars().getVariablesSet());
      model.checkPostInsertion();
      model.checkVariables(VariablesSet{A, B, C});

      CHECK(model.getAllFactors().size() == 2);
      CHECK(model.getConstFactors().size() == 0);
      CHECK(model.getTunableFactors().size() == 2);
      const auto clusters = model.getTunableClusters();
      CHECK(model.getTuners().size() == 1);
      CHECK(clusters.size() == 1);

      VisitorConst<train::FactorExponentialPtr, train::TunableClusters>(
          [](const train::FactorExponentialPtr &) {
            throw std::runtime_error{"TunableClusters was expected"};
          },
          [](const train::TunableClusters &cl) { CHECK(cl.size() == 2); })
          .visit(clusters.front());
    }
  }
}

TEST_CASE("tunable factors multiple insertions", "[insertion]") {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");
  auto C = make_variable(2, "C");
  auto D = make_variable(2, "D");

  FactorsTunableManagerTest model;

  auto factor_AB = make_corr_expfactor(A, B, 1.f);
  auto factor_BC = make_corr_expfactor(B, C, 1.f);
  factor::FactorExponential factor_D(factor::Factor{Group{VariablesSoup{D}}});

  model.copyTunableFactor(factor_AB);
  model.copyConstFactor(factor_BC);
  model.copyTunableFactor(factor_D);
  model.checkVariables(VariablesSet{A, B, C, D});

  CHECK(model.getAllFactors().size() == 3);
  CHECK(model.getConstFactors().size() == 1);
  CHECK(model.getTunableFactors().size() == 2);
  CHECK(model.getTuners().size() == 2);

  const auto &state = model.state();
  CHECK(state.clusters.size() == 2);
  CHECK((state.clusters.front().nodes.size() == 3 ||
         state.clusters.back().nodes.size() == 3));
  CHECK((state.clusters.front().nodes.size() == 1 ||
         state.clusters.back().nodes.size() == 1));
}

TEST_CASE("check bad factor insertions are refused", "[insertion]") {
  auto A = make_variable(2, "A");
  auto B = make_variable(2, "B");

  FactorsTunableManagerTest model;

  auto to_insert1 = make_corr_factor_ptr(A, B);
  model.addConstFactor(to_insert1);

  SECTION("factor connecting same variables") {
    auto to_insert2 = make_corr_factor_ptr(A, B);
    CHECK_THROWS_AS(model.addConstFactor(to_insert2), Error);
  }

  SECTION("factor referring to a bad variable") {
    auto A_bis = make_variable(2, "A");

    SECTION("bad unary factor") {
      auto to_insert2 = std::make_shared<Factor>(Group{VariablesSoup{A_bis}});
      CHECK_THROWS_AS(model.addConstFactor(to_insert2), Error);
    }

    SECTION("bad binary factor") {
      auto C = make_variable(2, "C");
      auto to_insert2 = make_corr_factor_ptr(A_bis, C);
      CHECK_THROWS_AS(model.addConstFactor(to_insert2), Error);
    }
  }
}

} // namespace EFG::test
