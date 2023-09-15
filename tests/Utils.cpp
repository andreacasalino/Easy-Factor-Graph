#include "Utils.h"
#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/structure/GibbsSampler.h>

#include <math.h>
#include <sstream>

namespace EFG::test {
bool almost_equal_fnct(const factor::Function &a, const factor::Function &b) {
  bool res = true;
  a.forEachCombination<true>([&b, &res](const auto &comb, float img) {
    if (!almost_equal(img, b.findTransformed(comb), 0.001f)) {
      res = false;
    }
  });
  return res;
}

categoric::Group make_group(const std::vector<std::size_t> &sizes) {
  if (sizes.empty()) {
    throw Error{"empty sizes"};
  }
  categoric::Group group{categoric::make_variable(sizes.front(), "V0")};
  std::size_t count = 1;
  std::for_each(sizes.begin() + 1, sizes.end(), [&group, &count](auto size) {
    group.add(categoric::make_variable(size, "V" + std::to_string(count++)));
  });
  return group;
}

factor::Factor make_corr_factor(const categoric::VariablePtr &first,
                                const categoric::VariablePtr &second) {
  return factor::Factor(categoric::Group{first, second},
                        factor::Factor::SimplyCorrelatedTag{});
}

std::shared_ptr<factor::Factor>
make_corr_factor_ptr(const categoric::VariablePtr &first,
                     const categoric::VariablePtr &second) {
  return std::make_shared<factor::Factor>(
      categoric::Group{first, second}, factor::Factor::SimplyCorrelatedTag{});
}

factor::FactorExponential
make_corr_expfactor(const categoric::VariablePtr &first,
                    const categoric::VariablePtr &second, float w) {
  auto factor = make_corr_factor(first, second);
  return factor::FactorExponential(factor, w);
}

std::shared_ptr<factor::FactorExponential>
make_corr_expfactor_ptr(const categoric::VariablePtr &first,
                        const categoric::VariablePtr &second, float w) {
  auto factor = make_corr_factor(first, second);
  return std::make_shared<factor::FactorExponential>(factor, w);
}

std::vector<float> make_prob_distr(const std::vector<float> &values) {
  float coeff = 0;
  for (const auto &val : values) {
    coeff += val;
  }
  coeff = 1.f / coeff;
  std::vector<float> result = values;
  for (auto &val : result) {
    val *= coeff;
  }
  return result;
}

void setAllImages(factor::Factor &subject, float img) {
  categoric::GroupRange range{subject.function().vars()};
  categoric::for_each_combination(
      range, [&](const auto &comb) { subject.set(comb, img); });
}

CombinationsAndProbabilities
compute_combinations_and_probs(const strct::FactorsAware &model) {
  categoric::Group all_vars(model.getAllVariables());
  CombinationsAndProbabilities result;
  std::vector<const factor::Immutable *> factors;
  for (const auto &factor : model.getAllFactors()) {
    factors.push_back(factor.get());
  }
  factor::Factor merged_sorted =
      factor::Factor{factors}.cloneWithPermutedGroup(all_vars);
  result.probs = merged_sorted.getProbabilities();
  result.combinations.reserve(all_vars.size());
  categoric::GroupRange range(all_vars);
  categoric::for_each_combination(
      range, [&combinations = result.combinations](const auto &comb) {
        combinations.emplace_back(comb);
      });
  return result;
}

train::TrainSet make_good_trainset(const strct::FactorsAware &model,
                                   std::size_t samples) {
  auto &&[probs, combinations] = compute_combinations_and_probs(model);
  std::vector<std::vector<std::size_t>> sampled;
  sampled.reserve(samples);
  strct::UniformSampler sampler;
  sampler.resetSeed(0);
  while (sampled.size() != samples) {
    auto pos = sampler.sampleFromDiscrete(probs);
    sampled.emplace_back(combinations[pos]);
  }
  return train::TrainSet{sampled};
}

LikelihoodGetter::LikelihoodGetter(const strct::FactorsAware &model)
    : vars{model.getAllVariables()} {
  const auto &factors = model.getAllFactors();
  finders.reserve(factors.size());
  for (const auto &factor : factors) {
    finders.emplace_back(factor->makeFinder(vars.getVariables()));
  }
};

float LikelihoodGetter::getLogActivation(
    const std::vector<std::size_t> &c) const {
  float res = 0.f;
  for (const auto &finder : finders) {
    res += logf(finder.findTransformed(c));
  }
  return res;
};

float LikelihoodGetter::getLogLikeliHood(
    const EFG::train::TrainSet::Iterator &combinations) {
  float Z = 0.f;
  {
    categoric::GroupRange range(categoric::Group{vars});
    for_each_combination(range, [this, &Z](const auto &comb) {
      Z += this->getLogActivation(comb);
    });
  }
  float lkl = 0.f, coeff = 1.f / static_cast<float>(combinations.size());
  combinations.forEachSample([this, &lkl, &coeff](const auto &comb) {
    lkl += coeff * this->getLogActivation(comb);
  });
  return lkl - Z;
}

} // namespace EFG::test
