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

struct GenericCopyTag {};
constexpr GenericCopyTag GENERIC_COPY_TAG = GenericCopyTag{};

class Factor : public DistributionConcrete, public DistributionSetter {
public:
  /**
   * @brief The variables set representing this factor is copied from the passed
   * one.
   * All the combinations instanciated in the passed factor, are copied in the
   * combinations map of the factor to build, assigning the images values
   * obtained by evaluating the passed factor.
   */
  Factor(const Distribution &to_clone, const GenericCopyTag &GENERIC_COPY_TAG);

  Factor(const Factor &o);

  /**
   * @brief The variables set representing this factor is assumed equal to the
   * passed one.
   * No combinations is instanciated, implicitly assuming all the images equal
   * to 0.
   */
  Factor(const categoric::Group &vars);

  /**
   * @brief A simply correlating factor is built.
   * All the variables in the passed group should have the same size.
   * The images pertaining to the combinations having all values equal, are
   * assumed equal to 1, all the others to 0.
   * For instance assume to pass a variable set equal to: {<A: size 3>, <B: size
   * 3>, <C: size 3>}. Then, the following combinations map is built:
   * <0,0,0> -> 1
   * <0,0,1> -> 0
   * <0,0,2> -> 0
   *
   * <0,1,0> -> 0
   * <0,1,1> -> 0
   * <0,1,2> -> 0
   *
   * <0,2,0> -> 0
   * <0,2,1> -> 0
   * <0,2,2> -> 0
   *
   * <1,0,0> -> 0
   * <1,0,1> -> 0
   * <1,0,2> -> 0
   *
   * <1,1,0> -> 0
   * <1,1,1> -> 1
   * <1,1,2> -> 0
   *
   * <1,2,0> -> 0
   * <1,2,1> -> 0
   * <1,2,2> -> 0
   *
   * <2,0,0> -> 0
   * <2,0,1> -> 0
   * <2,0,2> -> 0
   *
   * <2,1,0> -> 0
   * <2,1,1> -> 0
   * <2,1,2> -> 0
   *
   * <2,2,0> -> 0
   * <2,2,1> -> 0
   * <2,2,2> -> 1
   */
  Factor(const categoric::Group &vars, const UseSimpleCorrelation &);

  /**
   * @brief Similar to Factor(const categoric::Group &, const
   * UseSimpleCorrelation &), but considering a simple anti-correlation.
   * Therefore, to all combinations having all equal values, an image equal to 0
   * is assigned. All the other ones, are assigned a value equal to 1.
   * For instance assume to pass a variable set equal to: {<A: size 2>, <B: size
   * 2>}. Then, the following combinations map is built:
   * <0,0,0> -> 0
   * <0,0,1> -> 1
   * <0,0,2> -> 1
   *
   * <0,1,0> -> 1
   * <0,1,1> -> 1
   * <0,1,2> -> 1
   *
   * <0,2,0> -> 1
   * <0,2,1> -> 1
   * <0,2,2> -> 1
   *
   * <1,0,0> -> 1
   * <1,0,1> -> 1
   * <1,0,2> -> 1
   *
   * <1,1,0> -> 1
   * <1,1,1> -> 0
   * <1,1,2> -> 1
   *
   * <1,2,0> -> 1
   * <1,2,1> -> 1
   * <1,2,2> -> 1
   *
   * <2,0,0> -> 1
   * <2,0,1> -> 1
   * <2,0,2> -> 1
   *
   * <2,1,0> -> 1
   * <2,1,1> -> 1
   * <2,1,2> -> 1
   *
   * <2,2,0> -> 1
   * <2,2,1> -> 1
   * <2,2,2> -> 0
   */
  Factor(const categoric::Group &vars, const UseSimpleAntiCorrelation &);

  /**
   * @brief Builds the factor by merging all the passed factors.
   * The variables set representing this factor is obtained as the union of the
   * all the variables sets of the passed distribution.
   */
  template <typename... Distributions>
  Factor(const Distribution &first, const Distribution &second,
         const Distributions &...others)
      : Factor(pack_factors(first, second, others...)) {}

  /**
   * @brief Generates a Factor similar to this one, permuting the group of
   * variables.
   * @param the new variables group order to assume
   * @return the permuted variables factor
   * @throw in case new_order.getVariablesSet() !=
   * this->getVariables().getVariablesSet()
   */
  Factor cloneWithPermutedGroup(const categoric::Group &new_order) const;

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
