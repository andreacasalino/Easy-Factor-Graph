/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Factor.h>

namespace EFG::distribution {
class UnaryFactor : public distribution::Factor {
public:
  // all ones are assumed
  UnaryFactor(const categoric::VariablePtr &var);

  UnaryFactor(const std::vector<DistributionCnstPtr> &factors);

  const categoric::VariablePtr &getVariable() const { return variable; }

  void merge(const Distribution &to_merge);

  void normalize();

protected:
  struct DontFillDomainTag {};
  static const DontFillDomainTag DONT_FILL_DOMAIN_TAG;
  UnaryFactor(const categoric::VariablePtr &var, const DontFillDomainTag &);

private:
  categoric::VariablePtr variable;
};

class Message : public UnaryFactor {
public:
  Message(const Distribution &binary_factor, const UnaryFactor &unary_factor);

  Message(const Distribution &binary_factor, const categoric::VariablePtr &var);
};

class Evidence : public UnaryFactor {
public:
  Evidence(const Distribution &binary_factor,
           const categoric::VariablePtr &evidence_var,
           const std::size_t evidence);
};
} // namespace EFG::distribution

namespace EFG::strct {
std::unique_ptr<distribution::Evidence>
make_evidence(const distribution::Distribution &binary_factor,
              const categoric::VariablePtr &evidence_var,
              const std::size_t evidence);
}
