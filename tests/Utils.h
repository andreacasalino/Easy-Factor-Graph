#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/factor/Factor.h>
#include <EasyFactorGraph/factor/FactorExponential.h>
#include <EasyFactorGraph/structure/bases/FactorsAware.h>
#include <EasyFactorGraph/trainable/TrainSet.h>

#include <chrono>
#include <cmath>
#include <math.h>

namespace EFG::test {
template <typename T> bool almost_equal(T a, T b, T tollerance) {
  return std::abs(a - b) < tollerance;
}

template <typename IterableA, typename IterableB>
bool almost_equal_it(const IterableA &a, const IterableB &b, float tollerance) {
  if (a.size() != b.size()) {
    return false;
  }
  auto it_a = a.begin();
  auto it_b = b.begin();
  for (; it_a != a.end(); ++it_a, ++it_b) {
    if (!almost_equal(*it_a, *it_b, tollerance)) {
      return false;
    }
  }
  return true;
}

bool almost_equal_fnct(const factor::Function &a, const factor::Function &b);

categoric::Group make_group(const std::vector<std::size_t> &sizes);

factor::Factor make_corr_factor(const categoric::VariablePtr &first,
                                const categoric::VariablePtr &second);

std::shared_ptr<factor::Factor>
make_corr_factor_ptr(const categoric::VariablePtr &first,
                     const categoric::VariablePtr &second);

factor::FactorExponential
make_corr_expfactor(const categoric::VariablePtr &first,
                    const categoric::VariablePtr &second, float w);

std::shared_ptr<factor::FactorExponential>
make_corr_expfactor_ptr(const categoric::VariablePtr &first,
                        const categoric::VariablePtr &second, float w);

std::vector<float> make_prob_distr(const std::vector<float> &values);

void setAllImages(factor::Factor &subject, float img);

struct CombinationsAndProbabilities {
  std::vector<float> probs;
  std::vector<std::vector<std::size_t>> combinations;
};
CombinationsAndProbabilities
compute_combinations_and_probs(const strct::FactorsAware &model);

train::TrainSet make_good_trainset(const strct::FactorsAware &model,
                                   std::size_t samples);

template <typename Pred> std::chrono::nanoseconds measure_time(Pred &&pred) {
  auto tic = std::chrono::high_resolution_clock::now();
  pred();
  return std::chrono::duration_cast<std::chrono::nanoseconds>(
      std::chrono::high_resolution_clock::now() - tic);
}

class LikelihoodGetter {
public:
  LikelihoodGetter(const strct::FactorsAware &model);

  float getLogActivation(const std::vector<std::size_t> &c) const;

  float getLogLikeliHood(const EFG::train::TrainSet::Iterator &combinations);

private:
  categoric::Group vars;
  std::vector<factor::ImageFinder> finders;
};

} // namespace EFG::test
