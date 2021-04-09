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

    class Distribution {
    friend class DistributionIterator;
    public:
        virtual ~Distribution() = default;

        inline const categoric::Group& getGroup() const { return *this->group; };

        DistributionIterator getIterator() const;

        // returns 0 in case such a combination does not exist
        float find(const Combination& comb) const;

        // group size should be bigger and contain this->variables
        // returns <nullptr, 0> in case such a combination does not exist
        std::pair<const Combination*, float> find(const Combination& comb, const categoric::Group& group) const;

    protected:
        Distribution() = default;

        std::unique_ptr<categoric::Group> group;
        std::shared_ptr<std::map<Combination, float>> values;
        EvaluatorPtr evaluator;
    };

    typedef std::shared_ptr<Distribution> DistributionPtr;
}

#endif
