/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/distribution/Factor.h>

namespace EFG::distribution {
class MessageSum : public Factor {
public:
  MessageSum(const distribution::Distribution &factor,
             const categoric::VariablePtr &toMarginalize);
};

class MessageMAP : public Factor {
public:
  MessageMAP(const distribution::Distribution &factor,
             const categoric::VariablePtr &toMarginalize);
};
} // namespace EFG::distribution
