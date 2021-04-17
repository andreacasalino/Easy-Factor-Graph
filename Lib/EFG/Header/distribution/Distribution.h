/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_DISTRIBUTION_H
#define EFG_DISTRIBUTION_DISTRIBUTION_H

#include <categoric/Group.h>
#include <distribution/Evaluator.h>
#include <Combination.h>
#include <map>

namespace EFG::distribution {
    class DistributionIterator;
    class DistributionFinder;

    class Distribution {
    friend class DistributionIterator;
    friend class DistributionFinder;
    public:
        virtual ~Distribution() = default;

        inline const categoric::Group& getGroup() const { return *this->group; };

        DistributionIterator getIterator() const;

        // returns 0 in case such a combination does not exist
        float find(const Combination& comb) const;
        float findRaw(const Combination& comb) const;

        DistributionFinder getFinder(const std::set<categoric::VariablePtr>& containingGroup) const;

        // same order of combinations obtained with Range(getGroup) is returned
        std::vector<float> getProbabilities() const;

    protected:
        Distribution() = default;

        void checkCombination(const Combination& comb, const float& value) const;

        std::unique_ptr<categoric::Group> group;
        std::shared_ptr<std::map<Combination, float>> values;
        EvaluatorPtr evaluator;
    };

    typedef std::shared_ptr<Distribution> DistributionPtr;

    template<typename F, typename ... Args>
    std::shared_ptr<F> makeFactor(Args ... args) {
        return std::make_shared<F>(args...);
    };
}

#endif
