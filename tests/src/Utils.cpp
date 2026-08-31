#include <EasyFactorGraph/misc/UniformSampler.h>

#include <Utils.h>

#include <algorithm>

namespace EFG::test {
bool same_sign(float a, float b) {
  if (std::abs(a) < 1e-3 && std::abs(b) < 1e-3) {
    return true;
  } else {
    return 0 < a * b;
  }
}

factor::UnaryFactorExponential
make_unary_exp_factor(float w, const std::vector<float> &domain) {
  factor::UnaryFactorExponential res{domain};
  res.trsfm.setWeight(w);
  return res;
}

std::vector<float> make_prob_distr(std::vector<float> distr) {
  float sum{0};
  for (auto val : distr) {
    sum += val;
  }
  for (auto &val : distr) {
    val /= sum;
  }
  return distr;
}

void add_corr_expfactor(structure::ModelBuilder &res, float w,
                        std::size_t var_a, std::size_t var_b) {
  auto var_size = res.get_seed().variables_sizes[var_a];
  auto factor = factor::make_exp_simply_correlated<2>(var_size, w);
  res.add_tunable_binary_factor(std::move(factor), var_a, var_b);
}

void add_indicator_expfactor(structure::ModelBuilder &res, float w,
                             std::size_t var) {
  auto var_size = res.get_seed().variables_sizes[var];
  std::vector<float> support;
  support.resize(var_size, 0);
  support[0] = 1.f;
  factor::UnaryFactorExponential factor{std::move(support)};
  factor.trsfm.setWeight(w);
  res.add_unary_factor(std::move(factor), var);
}
} // namespace EFG::test
