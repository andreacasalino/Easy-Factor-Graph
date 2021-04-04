/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_FACTOR_MUTABLE_H
#define EFG_DISTRIBUTION_FACTOR_MUTABLE_H

#include <distribution/DistributionMutable.h>
#include <distribution/FactorConst.h>
#include <distribution/FactorExponentialConst.h>

namespace EFG::distribution {
  template<typename FactorT_, typename MutableT_>
  class FactorMutable
      : public FactorT_
      , public MutableT_ {
  static_assert(std::is_base_of<DistributionBase, FactorT_>::value , "FactorT_ should be derived from DistributionBase");
  static_assert(std::is_base_of<DistributionSettable, MutableT_>::value , "MutableT_ should be derived from DistributionSettable");
      public:
      template<typename ... Args>
      FactorMutable(Args&&... args)
        : FactorT_(std::forward<Args>(args)...) {
      };
  };

  typedef FactorMutable<FactorConst, DistributionModifiable> Factor;

  typedef FactorMutable<FactorExponentialConst, DistributionSettable> FactorExponential;
}

#endif
