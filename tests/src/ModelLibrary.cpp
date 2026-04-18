#include "ModelLibrary.h"
#include "Utils.h"

namespace EFG::test {
LineModel::LineModel(categoric::VarStateSize var_size, float w)
    : EFG::model::RandomField{make(var_size, std::span<float>{&w, 1})} {}

LineModel::LineModel(categoric::VarStateSize var_size,
                     const std::vector<float> &w)
    : EFG::model::RandomField{make(var_size, w)} {}

structure::ModelSeed LineModel::make(categoric::VarStateSize var_size,
                                     std::span<const float> w) {
  structure::ModelBuilder res;

  auto var_prev = res.make_variable(var_size);
  for (auto ww : w) {
    auto var_current = res.make_variable(var_size);
    add_corr_expfactor(res, ww, var_prev, var_current);
    var_prev = var_current;
  }

  return structure::ModelBuilder::build(std::move(res));
}

structure::ModelSeed EvidenceTestModel::make() {
  structure::ModelBuilder res;

  auto make_level_ = [&](std::size_t len) {
    std::vector<std::size_t> level;
    for (std::size_t k = 0; k < len; ++k) {
      level.push_back(res.make_variable(2));
    }
    return level;
  };

  auto aLevel = make_level_(N);
  auto mLevel = make_level_(N);
  auto lLevel = make_level_(N);

  auto make_factor_ = [&](std::size_t index_a, std::size_t index_b) {
    res.add_binary_factor(factor::make_simply_correlated<2>(2), index_a,
                          index_b);
  };

  for (std::size_t k = 0; k < N; ++k) {
    make_factor_(aLevel[k], mLevel[k]);
    make_factor_(lLevel[k], mLevel[k]);
  }
  for (std::size_t k = 1; k < N; ++k) {
    make_factor_(aLevel[k - 1], mLevel[k]);
    make_factor_(lLevel[k - 1], mLevel[k]);
  }

  return structure::ModelBuilder::build(std::move(res));
}

namespace {
std::vector<std::size_t> make_level(std::size_t from, std::size_t to) {
  std::vector<std::size_t> res;
  res.reserve(to - from);
  for (std::size_t k = from; k < to; ++k) {
    res.push_back(k);
  }
  return res;
}
} // namespace

EvidenceTestModel::EvidenceTestModel()
    : EFG::model::Model<structure::QueryManager, structure::EvidenceSetManager,
                        structure::EvidenceRemoveManager>{make()},
      uVars{make_level(0, N)}, mVars{make_level(N, 2 * N)}, lVars{make_level(
                                                                2 * N, 3 * N)} {
}

bool EvidenceTestModel::clusterExists(
    const std::vector<std::size_t> &vars) const {
  const auto &context = this->getStructure();
  return std::find_if(
             context.clusters.begin(), context.clusters.end(),
             [&](const structure::HiddenCluster &cluster) {
               if (vars.size() != cluster.variables.size()) {
                 return false;
               }
               return std::all_of(vars.begin(), vars.end(), [&](auto index) {
                 return std::find(cluster.variables.begin(),
                                  cluster.variables.end(),
                                  index) != cluster.variables.end();
               });
             }) != context.clusters.end();
}

bool EvidenceTestModel::allHiddenCluserExists() const {
  std::vector<std::size_t> all_vars;
  for (std::size_t k = 0; k < 3 * N; ++k) {
    all_vars.push_back(k);
  }
  return clusterExists(all_vars);
}

SimpleTree::SimpleTree() : EFG::model::RandomField{make()} {}

structure::ModelSeed SimpleTree::make() {
  structure::ModelBuilder res;

  std::size_t var_a = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "A"});
  std::size_t var_b = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "B"});
  std::size_t var_c = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "C"});
  std::size_t var_d = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "D"});
  std::size_t var_e = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "E"});

  add_corr_expfactor(res, alfa, var_a, var_b);
  add_corr_expfactor(res, beta, var_b, var_c);
  add_corr_expfactor(res, gamma, var_b, var_d);
  add_corr_expfactor(res, eps, var_d, var_e);

  return structure::ModelBuilder::build(std::move(res));
}

ComplexTree::ComplexTree() : EFG::model::RandomField{make()} {}

structure::ModelSeed ComplexTree::make() {
  structure::ModelBuilder res;

  for (std::size_t k = 0; k < 13; ++k) {
    res.make_variable(2);
  }

  std::pair<std::size_t, std::size_t> edges[] = {
      {1, 4}, {2, 4}, {3, 5},  {4, 6},  {4, 7},  {5, 7},
      {5, 8}, {6, 9}, {6, 10}, {7, 11}, {8, 12}, {8, 13},
  };

  const float w = 1.f;
  for (auto [a, b] : std::span<std::pair<std::size_t, std::size_t>>{edges}) {
    add_corr_expfactor(res, w, a - 1, b - 1);
  }

  return structure::ModelBuilder::build(std::move(res));
}

SimpleLoopy::SimpleLoopy() : EFG::model::RandomField{make()} {}

structure::ModelSeed SimpleLoopy::make() {
  structure::ModelBuilder res;

  std::size_t var_a = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "A"});
  std::size_t var_b = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "B"});
  std::size_t var_c = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "C"});
  std::size_t var_d = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "D"});
  std::size_t var_e = res.make_variable(
      2, structure::ModelBuilder::VariableDescription{.label = "E"});

  add_corr_expfactor(res, w, var_a, var_b);
  add_corr_expfactor(res, w, var_b, var_c);
  add_corr_expfactor(res, w, var_b, var_d);
  add_corr_expfactor(res, w, var_c, var_d);
  add_corr_expfactor(res, w, var_e, var_d);

  return structure::ModelBuilder::build(std::move(res));
}

ComplexLoopy::ComplexLoopy() : EFG::model::RandomField{make()} {}

structure::ModelSeed ComplexLoopy::make() {
  structure::ModelBuilder res;

  for (std::size_t k = 0; k < 8; ++k) {
    res.make_variable(2);
  }

  std::pair<std::size_t, std::size_t> edges[] = {
      {1, 2}, {2, 4}, {2, 3}, {3, 4}, {4, 5},
      {3, 5}, {4, 6}, {5, 7}, {6, 7}, {7, 8},
  };

  const float w = 1.f;
  for (auto [a, b] : std::span<std::pair<std::size_t, std::size_t>>{edges}) {
    add_corr_expfactor(res, w, a - 1, b - 1);
  }

  return structure::ModelBuilder::build(std::move(res));
}

MatrixLoopy::MatrixLoopy(std::size_t size)
    : EFG::model::RandomField{make(size)} {}

structure::ModelSeed MatrixLoopy::make(std::size_t size) {
  structure::ModelBuilder res;

  for (std::size_t r = 0; r < size; ++r) {
    for (std::size_t c = 0; c < size; ++c) {
      res.make_variable(2);
    }
  }

  auto get_var_ = [&size](std::size_t r, std::size_t c) {
    return size * r + c;
  };

  const float w = 1.f;
  for (std::size_t c = 1; c < size; ++c) {
    add_corr_expfactor(res, w, c - 1, c);
  }
  for (std::size_t r = 1; r < size; ++r) {
    add_corr_expfactor(res, w, get_var_(r, 0), get_var_(r - 1, 0));
    for (std::size_t c = 1; c < size; ++c) {
      add_corr_expfactor(res, w, get_var_(r, c), get_var_(r - 1, c));
      add_corr_expfactor(res, w, get_var_(r, c - 1), get_var_(r, c));
    }
  }

  return structure::ModelBuilder::build(std::move(res));
}

ChainModel::ChainModel(std::size_t len, float wXY, float wYY)
    : EFG::model::RandomField{make(len, wXY, wYY)} {
  this->setEvidences(structure::Evidence{len, 0});
  this->setEvidences(structure::Evidence{len + 1, 1});
  this->setEvidences(structure::Evidence{len + 2, 0});
  this->setEvidences(structure::Evidence{len + 3, 1});
}

structure::ModelSeed ChainModel::make(std::size_t len, float wXY, float wYY) {
  structure::ModelBuilder res;

  for (std::size_t k = 0; k < len; ++k) {
    res.make_variable(2); // Y_k
    res.make_variable(2); // X_k
  }

  for (std::size_t k = 0; k < len; ++k) {
    add_corr_expfactor(res, wXY, k, k + len);
  }
  for (std::size_t k = 1; k < len; ++k) {
    add_corr_expfactor(res, wYY, k - 1, k);
  }

  factor::UnaryFactorExponential u{{0, 1.f}};
  u.trsfm.setWeight(wYY);
  res.add_unary_factor(std::move(u), 0);

  return structure::ModelBuilder::build(std::move(res));
}

/*
namespace {
std::string make_scalable_var_name(std::size_t counter) {
return "var_" + std::to_string(counter);
}

struct BinaryTreeContext {
model::RandomField &subject;
std::size_t var_size;
float w;
bool loopy;
};
void fill_scalable_model(const BinaryTreeContext &ctxt,
                       std::size_t remaining_levels, std::size_t parent,
                       std::size_t &counter) {
if (0 == remaining_levels) {
  return;
}

categoric::VariablePtr parent_var;
if (0 == counter) {
  ++counter;
  parent = counter;
  parent_var =
      categoric::make_variable(ctxt.var_size, make_scalable_var_name(parent));

} else {
  parent_var = ctxt.subject.findVariable(make_scalable_var_name(parent));
}

auto add_child = [&]() {
  ++counter;
  const auto new_var_id = counter;
  auto new_var = categoric::make_variable(ctxt.var_size,
                                          make_scalable_var_name(new_var_id));
  ctxt.subject.addTunableFactor(
      make_corr_expfactor_ptr(parent_var, new_var, ctxt.w));

  fill_scalable_model(ctxt, remaining_levels - 1, new_var_id, counter);
  return new_var;
};

auto left_var = add_child();
auto right_var = add_child();

if (ctxt.loopy) {
  ctxt.subject.addTunableFactor(
      make_corr_expfactor_ptr(left_var, right_var, ctxt.w));
}
}
} // namespace

ScalableModel::ScalableModel(std::size_t size, std::size_t var_size,
                           const bool loopy) {
if (0 == size) {
  throw Error{"Invalid depth"};
}
std::size_t counter = 0;
fill_scalable_model(BinaryTreeContext{*this, var_size, 1.f, loopy}, size, 0,
                    counter);
}

categoric::VariablePtr ScalableModel::root() const {
return findVariable(make_scalable_var_name(1));
}

categoric::VariablePtr ScalableModel::nonRoot() const {
return findVariable(make_scalable_var_name(2));
}
*/
} // namespace EFG::test
