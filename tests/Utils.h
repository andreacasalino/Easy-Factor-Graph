#pragma once

#include <EasyFactorGraph/distribution/Factor.h>
#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/structure/ConnectionsManager.h>

#include <math.h>
#include <memory>

namespace EFG::test {
distribution::Factor make_corr_factor(const categoric::VariablePtr &first,
                                      const categoric::VariablePtr &second);

std::shared_ptr<distribution::Factor>
make_corr_factor2(const categoric::VariablePtr &first,
                  const categoric::VariablePtr &second);

distribution::FactorExponential
make_corr_expfactor(const categoric::VariablePtr &first,
                    const categoric::VariablePtr &second, const float w);

std::shared_ptr<distribution::FactorExponential>
make_corr_expfactor2(const categoric::VariablePtr &first,
                     const categoric::VariablePtr &second, const float w);

bool almost_equal(const float a, const float b, const float tollerance);

bool almost_equal(const std::vector<float> &a, const std::vector<float> &b,
                  const float tollerance);

class ProbDistribution {
public:
  ProbDistribution(const std::vector<float> &values);

  const std::vector<float> &operator*() const { return values_normalized; };

private:
  const std::vector<float> values_normalized;
};

struct CombinationsAndProbabilities {
  std::vector<float> probs;
  std::vector<categoric::Combination> combinations;
};
CombinationsAndProbabilities
compute_combinations_and_probs(const strct::ConnectionsManager &model);
} // namespace EFG::test
