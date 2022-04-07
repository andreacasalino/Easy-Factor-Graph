/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Factor.h>

#include <type_traits>

namespace EFG::distribution {
class UnaryFactor : public distribution::Factor {
public:
  // all ones are assumed
  UnaryFactor(const categoric::VariablePtr &var);

  UnaryFactor(const std::vector<const distribution::Distribution *> &factors);

  const categoric::VariablePtr &getVariable() const { return variable; }

  void merge(const Distribution &to_merge);

  // check if this is actually used to normalized cached merged unaries
  void normalize();

protected:
  struct DontFillDomainTag {};
  static const DontFillDomainTag DONT_FILL_DOMAIN_TAG;
  UnaryFactor(const categoric::VariablePtr &var, const DontFillDomainTag &);

private:
  categoric::VariablePtr variable;
};

class Evidence : public UnaryFactor {
public:
  Evidence(const Distribution &binary_factor,
           const categoric::VariablePtr &evidence_var,
           const std::size_t evidence);
};

class MessageSUM : public UnaryFactor {
public:
  MessageSUM(const UnaryFactor &merged_unaries,
             const distribution::Distribution &binary_factor);
};

class MessageMAP : public UnaryFactor {
public:
  MessageMAP(const UnaryFactor &merged_unaries,
             const distribution::Distribution &binary_factor);
};

class Indicator : public UnaryFactor {
public:
  Indicator(const categoric::VariablePtr &var, const std::size_t value);
};
} // namespace EFG::distribution

namespace EFG::strct {
template <typename... Args>
std::unique_ptr<distribution::UnaryFactor> make_unary(Args &&...args) {
  return std::make_unique<distribution::UnaryFactor>(
      std::forward<Args>(args)...);
}

template <typename MessageT>
std::unique_ptr<MessageT>
make_message(const distribution::UnaryFactor &merged_unaries,
             const distribution::Distribution &binary_factor) {
  static_assert(std::is_same<MessageT, distribution::MessageMAP>{}() ||
                    std::is_same<MessageT, distribution::MessageSUM>{}(),
                "not a valid Message type");
  return std::make_unique<MessageT>(merged_unaries, binary_factor);
}

std::unique_ptr<distribution::Evidence>
make_evidence(const distribution::Distribution &binary_factor,
              const categoric::VariablePtr &evidence_var,
              const std::size_t evidence);
} // namespace EFG::strct
