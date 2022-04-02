/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/DistributionConcrete.h>
#include <EasyFactorGraph/distribution/DistributionSetter.h>

namespace EFG::distribution {
class BasicEvaluator;

struct UseSimpleCorrelation {};
constexpr UseSimpleCorrelation USE_SIMPLE_CORRELATION_TAG =
    UseSimpleCorrelation{};

struct UseSimpleAntiCorrelation {};
constexpr UseSimpleAntiCorrelation USE_SIMPLE_ANTI_CORRELATION_TAG =
    UseSimpleAntiCorrelation{};

class Factor : public DistributionConcrete, public DistributionSetter {
public:
  Factor(const Distribution &to_clone);

  // Factor(const Factor &o);
  Factor(Factor &&o);

  Factor(const categoric::Group &vars);

  Factor(const categoric::Group &vars, const UseSimpleCorrelation &);

  Factor(const categoric::Group &vars, const UseSimpleAntiCorrelation &);

  template <typename... Distributions>
  Factor(const Distribution &first, const Distribution &second,
         const Distributions &...others)
      : Factor(pack_factors(first, second, others...)) {}

protected:
  Factor(const categoric::Group &vars, const CombinationRawValuesMapPtr &map);

  Factor(const std::vector<const Distribution *> &factors);

private:
  template <typename... Distributions>
  static std::vector<const Distribution *>
  pack_factors(const Distributions &...factors) {
    std::vector<const Distribution *> recipient;
    pack_factors(recipient, factors...);
    return recipient;
  }

  template <typename... Distributions>
  static void pack_factors(std::vector<const Distribution *> &recipient,
                           const Distribution &to_pack,
                           const Distributions &...others) {
    pack_factors(recipient, to_pack);
    pack_factors(recipient, others...);
  }

  static void pack_factors(std::vector<const Distribution *> &recipient,
                           const Distribution &to_pack) {
    recipient.push_back(&to_pack);
  }
};
} // namespace EFG::distribution
