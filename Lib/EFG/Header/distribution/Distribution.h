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
        Distribution& operator=(const Distribution& o);

        // basic evaluator is assumed (even for exponential distribution)
        template<typename ... Distributions>
        Distribution(const Distribution* first, const Distribution* second, Distributions ... distr)
            : Distribution(pack(first, second, distr...)) {
        };

        // basic evaluator is assumed (even for exponential distribution)
        Distribution(const std::set<const Distribution*>& distr);

        DistributionIterator getIterator() const;

        inline const categoric::Group& getGroup() const { return this->variables; };

        ///////////////////// modifiers /////////////////////

        void clear();

        void add(const Combination& comb, const float& value);

        // put 0 for all not added combinations
        void emplaceEntireDomain();

        void setImageEntireDomain(const float& value);

        /////////////////////   query   /////////////////////

        // basic evaluator is assumed (even for exponential distribution)
        Distribution marginalize(const Combination& comb, const categoric::Group& evidences) const;

        // returns 0 in case such a combination does not exist
        float find(const Combination& comb) const;

        // group size should be bigger and contain this->variables
        // returns <nullptr, 0> in case such a combination does not exist
        std::pair<const Combination*, float> find(const Combination& comb, const categoric::Group& group) const;

    protected:
        Distribution(image::EvaluatorPtr evaluator, const categoric::Group& variables);

        template<typename ... Distributions>
        static std::set<const Distribution*> pack(const Distribution* first, const Distribution* second, Distributions ... distr) {
            std::set<const Distribution*> packed;
            pack(packed , first, second, distr...);
            return packed;
        };
        template<typename ... Distributions>
        static void pack(std::set<const Distribution*>& packed, const Distribution* first, Distributions ... distr) {
            packed.emplace(first);
            pack(packed, distr...);
        }
        static void pack(std::set<const Distribution*>& packed, const Distribution* first) {
            packed.emplace(first);
        }
        
        categoric::Group variables;
        image::EvaluatorPtr evaluator;
        std::map<Combination, float> values;
    };
}

#endif
