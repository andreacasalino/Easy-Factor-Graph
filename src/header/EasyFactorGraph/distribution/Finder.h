// /**
//  * Author:    Andrea Casalino
//  * Created:   01.01.2021
//  *
//  * report any bug to andrecasa91@gmail.com.
//  **/

// #ifndef EFG_DISTRIBUTION_DISTRIBUTION_FINDER_H
// #define EFG_DISTRIBUTION_DISTRIBUTION_FINDER_H

// #include <distribution/Distribution.h>

// namespace EFG::distribution {
//     /**
//      * @brief An object used to search for big combinations inside a
//      Distribution
//      */
//     class DistributionFinder {
//     public:
//         /**
//          * @param the reference distribution
//          * @param the variables referring to the combinations to search. This
//          kind of set should contain the subset of variables
//          * describing the domain of distribution
//          * @throw if some of the variables describing the distribution domain
//          are not contained in containingGroup
//          */
//         DistributionFinder(const Distribution& distribution, const
//         std::set<categoric::VariablePtr>& containingGroup);

//         DistributionFinder(const DistributionFinder&) = default;
//         DistributionFinder& operator=(const DistributionFinder&) = default;

//         /**
//          * @brief searches for matches. For example assume having built this
//          object with a containingGroup equal to <A,B,C,D> and the variables
//          describing the domain
//          * of the reference distribution equal to <B,D>. When passing comb ad
//          <0,1,2,0>, it searches for the <combination,image> pretaining to
//          this combination combination
//          * <B,D> = <1,0>.
//          * @param the combination to search, referring to the set of
//          variables passed when building this object.
//          * @return the pair <combination,image> of the the matching
//          combination. <nullptr,0> is returned in case such a combination was
//          not explicitly put in the distribution.
//          */
//         std::pair<const categoric::Combination*, float> find(const
//         categoric::Combination& comb) const;

//         /**
//          * @brief similar to DistributionFinder::find(...), but returning the
//          raw image value.
//          */
//         std::pair<const categoric::Combination*, float> findRaw(const
//         categoric::Combination& comb) const;

//     private:
//         std::shared_ptr<std::map<categoric::Combination, float>> values;
//         EvaluatorPtr evaluator;
//         std::vector<std::size_t> indicesContainingGroup;
//     };
// }

// #endif
