/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_DISTRIBUTION_DISTRIBUTION_H
#define EFG_DISTRIBUTION_DISTRIBUTION_H

#include <categoric/Group.h>
#include <distribution/Combination.h>
#include <map>
#include <distribution/image/Evaluator.h>

namespace EFG::distribution {
    class DistributionIterator;

    bool operator<(const Combination& a, const Combination& b);

    class Distribution {
    public:
        inline const categoric::Group& getGroup() const { return *this->getGroupPtr(); };

        virtual DistributionIterator getIterator() const = 0;

        // returns 0 in case such a combination does not exist
        float find(const Combination& comb) const;

        // group size should be bigger and contain this->variables
        // returns <nullptr, 0> in case such a combination does not exist
        std::pair<const Combination*, float> find(const Combination& comb, const categoric::Group& group) const;

    protected:
        Distribution() = default;

        virtual categoric::Group* getGroupPtr() const = 0;

        virtual std::map<Combination, float>* getValuesPtr() const = 0;

        virtual image::Evaluator* getEvaluatorPtr() const = 0;
    };
}



// #include <observers/MultiObservable.h>

// namespace EFG::distribution {
//     class DistributionIterator;

//     class Distribution : public observer::MultiObservable {
//     friend class DistributionIterator;
//     public:
//         Distribution(const categoric::Group& variables);

//         Distribution(const Distribution& o);
//         Distribution& operator=(const Distribution& o);

//         // basic evaluator is assumed (even for exponential distribution)
//         template<typename ... Distributions>
//         Distribution(const Distribution* first, const Distribution* second, Distributions ... distr)
//             : Distribution(pack(first, second, distr...)) {
//         };

//         // basic evaluator is assumed (even for exponential distribution)
//         Distribution(const std::set<const Distribution*>& distr);

//         DistributionIterator getIterator() const;

//         inline const categoric::Group& getGroup() const { return this->variables; };

//         ///////////////////// modifiers /////////////////////

//         void clear();

//         void add(const Combination& comb, const float& value);

//         // put 0 for all not added combinations
//         void emplaceEntireDomain();

//         void setImageEntireDomain(const float& value);

//         /////////////////////   query   /////////////////////

//         // basic evaluator is assumed (even for exponential distribution)
//         Distribution marginalize(const Combination& comb, const categoric::Group& evidences) const;

//         // returns 0 in case such a combination does not exist
//         float find(const Combination& comb) const;

//         // group size should be bigger and contain this->variables
//         // returns <nullptr, 0> in case such a combination does not exist
//         std::pair<const Combination*, float> find(const Combination& comb, const categoric::Group& group) const;

//     protected:
//         Distribution(image::EvaluatorPtr evaluator, const categoric::Group& variables);
        
//         categoric::Group variables;
//         image::EvaluatorPtr evaluator;
//         std::map<Combination, float> values;
//     };
// }

#endif
