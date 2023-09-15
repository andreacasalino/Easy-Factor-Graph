#include "ModelLibrary.h"
#include "Utils.h"

namespace EFG::test::library {
const float SimpleTree::alfa = 1.f;
const float SimpleTree::beta = 2.f;
const float SimpleTree::gamma = 1.f;
const float SimpleTree::eps = 1.5f;

SimpleTree::SimpleTree() {
  auto A = categoric::make_variable(2, "A");
  auto B = categoric::make_variable(2, "B");
  auto C = categoric::make_variable(2, "C");
  auto D = categoric::make_variable(2, "D");
  auto E = categoric::make_variable(2, "E");

  addTunableFactor(make_corr_expfactor_ptr(A, B, alfa));
  addTunableFactor(make_corr_expfactor_ptr(B, C, beta));
  addTunableFactor(make_corr_expfactor_ptr(B, D, gamma));
  addTunableFactor(make_corr_expfactor_ptr(D, E, eps));
}

ComplexTree::ComplexTree() {
  categoric::VariablesSoup vars;
  vars.push_back(nullptr);
  for (std::size_t k = 1; k <= 13; ++k) {
    vars.push_back(categoric::make_variable(2, "v" + std::to_string(k)));
  }

  const float w = 1.f;
  addTunableFactor(make_corr_expfactor_ptr(vars[1], vars[4], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[2], vars[4], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[3], vars[5], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[4], vars[6], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[4], vars[7], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[5], vars[7], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[5], vars[8], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[6], vars[9], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[6], vars[10], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[7], vars[11], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[8], vars[12], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[8], vars[13], w));
}

const float SimpleLoopy::w = 1.f;

SimpleLoopy::SimpleLoopy() {
  auto A = categoric::make_variable(2, "A");
  auto B = categoric::make_variable(2, "B");
  auto C = categoric::make_variable(2, "C");
  auto D = categoric::make_variable(2, "D");
  auto E = categoric::make_variable(2, "E");

  addTunableFactor(make_corr_expfactor_ptr(A, B, w));
  addTunableFactor(make_corr_expfactor_ptr(B, C, w));
  addTunableFactor(make_corr_expfactor_ptr(B, D, w));
  addTunableFactor(make_corr_expfactor_ptr(C, D, w));
  addTunableFactor(make_corr_expfactor_ptr(E, D, w));
}

ComplexLoopy::ComplexLoopy() {
  categoric::VariablesSoup vars;
  vars.push_back(nullptr);
  for (std::size_t k = 1; k <= 8; ++k) {
    vars.push_back(categoric::make_variable(2, "v" + std::to_string(k)));
  }

  const float w = 1.f;
  addTunableFactor(make_corr_expfactor_ptr(vars[1], vars[2], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[2], vars[4], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[2], vars[3], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[3], vars[4], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[4], vars[5], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[3], vars[5], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[4], vars[6], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[5], vars[7], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[6], vars[7], w));
  addTunableFactor(make_corr_expfactor_ptr(vars[7], vars[8], w));
}

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
} // namespace EFG::test::library
