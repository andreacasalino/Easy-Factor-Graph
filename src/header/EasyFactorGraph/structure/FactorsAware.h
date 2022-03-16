/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Factor.h>
#include <EasyFactorGraph/distribution/FactorExponential.h>
#include <EasyFactorGraph/structure/components/NodesAware.h>

namespace EFG::strct {
class StructureAware : virtual public NodesAware {
public:
  std::unordered_set<std::shared_ptr<const distribution::Factor>>
  getFactorsConst() const;

  std::unordered_set<std::shared_ptr<const distribution::FactorExponential>>
  getFactorsExpConst() const;

private:
  std::unordered_set<std::shared_ptr<const distribution::Factor>> factors_const;

  std::unordered_set<std::shared_ptr<const distribution::FactorExponential>>
      factors_exp_const;
};
} // namespace EFG::strct
