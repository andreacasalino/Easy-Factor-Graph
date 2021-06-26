/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <structure/ActivationEvaluator.h>
#include <algorithm>
#include <math.h>

namespace EFG::strct {
    ActivationEvaluator::ActivationEvaluator(const std::set<distribution::DistributionCnstPtr>& factors, const std::set<categoric::VariablePtr>& vars)
        : factors(factors) {
        std::for_each(factors.begin(), factors.end(), [this, &vars](const distribution::DistributionCnstPtr& d) {
            this->finders.emplace_back(d->getFinder(vars));
        });
    }

    float ActivationEvaluator::getLogActivation(const categoric::Combination& combination) const {
        float E = 0.f;
        std::for_each(this->finders.begin(), this->finders.end(), [&E, &combination](const distribution::DistributionFinder& f) {
            E += logf(f.find(combination).second);
        });
        return E;
    }
}
