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
    /**
     * @brief An object used to search for specific combinations-images inside a Distribution
     */
    class DistributionFinder {
    public:
        /**
         * @param the considered distribution
         * @param the variables referring to the combinations to search. It should be set containg the subset of variables
         * describing the domain of distribution
         */
        DistributionFinder(const Distribution& distribution, const std::set<categoric::VariablePtr>& containingGroup);

        DistributionFinder(const DistributionFinder&) = default;
        DistributionFinder& operator=(const DistributionFinder&) = default;

        /**
         * @brief searches for matches. For example assume containingGroup equal to <A,B,C,D> and the variables in the domain 
         * of the referring distribution equal to <B,D>. When passing <0,1,2,0>, it searches for the image referring to the combination
         * <A,C> = <0,2>.
         * @param the combination to search, referring to the set of variables passed when building this object.
         * @return the pair <combination,image> of the the combination matching. <nullptr,0> is returned in case such a combination was not explicitly put in the distribution
         */
        std::pair<const Combination*, float> find(const Combination& comb) const;

        /**
         * @brief similar to DistributionFinder::find(...), but returning the row image value.
         */
        std::pair<const Combination*, float> findRaw(const Combination& comb) const;

    private:
        std::shared_ptr<std::map<Combination, float>> values;
        EvaluatorPtr evaluator;
        std::vector<std::size_t> indicesContainingGroup;
    };
}

#endif
