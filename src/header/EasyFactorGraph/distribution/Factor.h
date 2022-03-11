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
  Factor(const categoric::Group &vars);

  Factor(const categoric::Group &vars, const UseSimpleCorrelation &);

  Factor(const categoric::Group &vars, const UseSimpleAntiCorrelation &);
};
} // namespace EFG::distribution
