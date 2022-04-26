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

  addTunableFactor(make_corr_expfactor2(A, B, alfa));
  addTunableFactor(make_corr_expfactor2(B, C, beta));
  addTunableFactor(make_corr_expfactor2(B, D, gamma));
  addTunableFactor(make_corr_expfactor2(D, E, eps));
}

ComplexTree::ComplexTree() {
  categoric::VariablesSoup vars;
  vars.push_back(nullptr);
  for (std::size_t k = 1; k <= 13; ++k) {
    vars.push_back(categoric::make_variable(2, "v" + std::to_string(k)));
  }

  const float w = 1.f;
  addTunableFactor(make_corr_expfactor2(vars[1], vars[4], w));
  addTunableFactor(make_corr_expfactor2(vars[2], vars[4], w));
  addTunableFactor(make_corr_expfactor2(vars[3], vars[5], w));
  addTunableFactor(make_corr_expfactor2(vars[4], vars[6], w));
  addTunableFactor(make_corr_expfactor2(vars[4], vars[7], w));
  addTunableFactor(make_corr_expfactor2(vars[5], vars[7], w));
  addTunableFactor(make_corr_expfactor2(vars[5], vars[8], w));
  addTunableFactor(make_corr_expfactor2(vars[6], vars[9], w));
  addTunableFactor(make_corr_expfactor2(vars[6], vars[10], w));
  addTunableFactor(make_corr_expfactor2(vars[7], vars[11], w));
  addTunableFactor(make_corr_expfactor2(vars[8], vars[12], w));
  addTunableFactor(make_corr_expfactor2(vars[8], vars[13], w));
}

const float SimpleLoopy::w = 1.f;

SimpleLoopy::SimpleLoopy() {
  auto A = categoric::make_variable(2, "A");
  auto B = categoric::make_variable(2, "B");
  auto C = categoric::make_variable(2, "C");
  auto D = categoric::make_variable(2, "D");
  auto E = categoric::make_variable(2, "E");

  addTunableFactor(make_corr_expfactor2(A, B, w));
  addTunableFactor(make_corr_expfactor2(B, C, w));
  addTunableFactor(make_corr_expfactor2(B, D, w));
  addTunableFactor(make_corr_expfactor2(C, D, w));
  addTunableFactor(make_corr_expfactor2(E, D, w));
}

ComplexLoopy::ComplexLoopy() {
  categoric::VariablesSoup vars;
  vars.push_back(nullptr);
  for (std::size_t k = 1; k <= 8; ++k) {
    vars.push_back(categoric::make_variable(2, "v" + std::to_string(k)));
  }

  const float w = 1.f;
  addTunableFactor(make_corr_expfactor2(vars[1], vars[2], w));
  addTunableFactor(make_corr_expfactor2(vars[2], vars[4], w));
  addTunableFactor(make_corr_expfactor2(vars[2], vars[3], w));
  addTunableFactor(make_corr_expfactor2(vars[3], vars[4], w));
  addTunableFactor(make_corr_expfactor2(vars[4], vars[5], w));
  addTunableFactor(make_corr_expfactor2(vars[3], vars[5], w));
  addTunableFactor(make_corr_expfactor2(vars[4], vars[6], w));
  addTunableFactor(make_corr_expfactor2(vars[5], vars[7], w));
  addTunableFactor(make_corr_expfactor2(vars[6], vars[7], w));
  addTunableFactor(make_corr_expfactor2(vars[7], vars[8], w));
}
} // namespace EFG::test::library
