/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Factor.h>

#include <type_traits>

namespace EFG::factor {
// The special factors defined here, are used internally to propagate the belief
// in the most optimized possible way.

class UnaryFactor : public Factor {
public:
  const categoric::VariablePtr &getVariable() const { return variable; }

  float diff(const UnaryFactor &o) const;

protected:
  UnaryFactor(FunctionPtr data);

  categoric::VariablePtr variable;
};

class MergedUnaries : public UnaryFactor {
public:
  // all ones are assumed
  MergedUnaries(const categoric::VariablePtr &var);

  MergedUnaries(const std::vector<const Immutable *> &factors);

  void merge(const Immutable &to_merge);

  // mostly for performance purpose
  void normalize();
};

class Evidence : public UnaryFactor {
public:
  Evidence(const Immutable &binary_factor,
           const categoric::VariablePtr &evidence_var, std::size_t evidence);
};

class Indicator : public UnaryFactor {
public:
  Indicator(const categoric::VariablePtr &var, std::size_t value);
};

class MessageSUM : public UnaryFactor {
public:
  MessageSUM(const UnaryFactor &merged_unaries, const Immutable &binary_factor);
};

class MessageMAP : public UnaryFactor {
public:
  MessageMAP(const UnaryFactor &merged_unaries, const Immutable &binary_factor);
};

} // namespace EFG::factor

namespace EFG::strct {
template <typename MessageT>
std::unique_ptr<MessageT>
make_message(const factor::UnaryFactor &merged_unaries,
             const factor::Immutable &binary_factor) {
  static_assert(std::is_same<MessageT, factor::MessageMAP>::value ||
                    std::is_same<MessageT, factor::MessageSUM>::value,
                "not a valid Message type");
  return std::make_unique<MessageT>(merged_unaries, binary_factor);
}
} // namespace EFG::strct
