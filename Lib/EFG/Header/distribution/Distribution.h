/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_DISTRIBUTION_H
#define EFG_DISTRIBUTION_DISTRIBUTION_H

#include <map>
#include <distribution/Combination.h>
#include <distribution/image/Evaluator.h>
#include <categoric/Group.h>
#include <observers/MultiObservable.h>

namespace EFG::distribution {
    class DistributionIterator;

    class Distribution : public observer::MultiObservable {
    friend class DistributionIterator;
    public:
        Distribution(const categoric::Group& variables);

        Distribution(const Distribution& o);
        Distribution& operator=(const Distribution&) = delete;

        // basic evaluator is assumed (even for exponential distribution)
        template<typename ... Distributions>
        Distribution(const Distribution& first, const Distribution& second, Distributions ... distr);

        // basic evaluator is assumed (even for exponential distribution)
        Distribution(const std::set<const Distribution*>& distr);

        // basic evaluator is assumed (even for exponential distribution)
        Distribution marginalize(const Combination& comb, const categoric::Group& evidences) const;

        void clear();

        void add(const Combination& comb, const float& value);

        // put 0 for all not added combinations
        void emplaceEntireDomain();

        void setImageEntireDomain(const float& value);

        DistributionIterator getIterator() const;

        // returns 0 in case such a combination does not exist
        float find(const Combination& comb) const;

        // group size should be bigger and contain this->variables
        // returns <nullptr, 0> in case such a combination does not exist
        std::pair<const Combination*, float> find(const Combination& comb, const categoric::Group& group) const;

        inline const categoric::Group& getGroup() const { return this->variables; };

    protected:
        Distribution(image::EvaluatorPtr evaluator, const categoric::Group& variables);
        
        categoric::Group variables;
        image::EvaluatorPtr evaluator;
        std::map<Combination, float> values;
    };
}

#endif
