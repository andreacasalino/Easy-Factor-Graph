/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_DISTRIBUTION_FINDER_H
#define EFG_DISTRIBUTION_DISTRIBUTION_FINDER_H

#include <distribution/Distribution.h>

namespace EFG::distribution {
    class DistributionFinder {
    public:
        DistributionFinder(const Distribution& distribution, const std::set<categoric::VariablePtr>& containingGroup);

        DistributionFinder(const DistributionFinder&) = default;
        DistributionFinder& operator=(const DistributionFinder&) = default;

        // group size should be bigger and contain this->variables
        // returns <nullptr, 0> in case such a combination does not exist
        std::pair<const Combination*, float> find(const Combination& comb) const;
        std::pair<const Combination*, float> findRaw(const Combination& comb) const;

    private:
        std::shared_ptr<std::map<Combination, float>> values;
        EvaluatorPtr evaluator;
        std::vector<std::size_t> indices;
    };
}

#endif
