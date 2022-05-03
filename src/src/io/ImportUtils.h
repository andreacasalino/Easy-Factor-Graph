/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/io/Utils.h>

namespace EFG::io {
struct ImportPredicate {
  ImportPredicate(const AdderPtrs &m) : model(m) {}

  const AdderPtrs model;

  struct FactorAndSharingGroup {
    std::shared_ptr<distribution::FactorExponential> factor;
    categoric::VariablesSet group;
  };
  std::vector<FactorAndSharingGroup> cumulated;
  void importCumulatedTunable() const;

  void importConst(const distribution::DistributionCnstPtr &factor);

  void
  importTunable(const std::shared_ptr<distribution::FactorExponential> &factor,
                const std::optional<categoric::VariablesSet> &sharing_group =
                    std::nullopt);
};
} // namespace EFG::io
