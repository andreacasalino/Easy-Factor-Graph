/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifndef EFG_STRUCTURE_ACTIVATION_EVALUATOR_H
#define EFG_STRUCTURE_ACTIVATION_EVALUATOR_H

#include <distribution/Distribution.h>
#include <distribution/DistributionFinder.h>
#include <list>

namespace EFG::strct {
    class ActivationEvaluator {
        friend class QueryHandler;
    public:
        float getLogActivation(const categoric::Combination& combination) const;

        inline const std::set<distribution::DistributionCnstPtr>& getFactors() const { return this->factors; };

    private:
        ActivationEvaluator(const std::set<distribution::DistributionCnstPtr>& factors, const std::set<categoric::VariablePtr>& vars);

        std::set<distribution::DistributionCnstPtr> factors;
        std::list<distribution::DistributionFinder> finders;
    };
}

#endif
