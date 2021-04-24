/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/DistributionFinder.h>
#include <Error.h>
#include <algorithm>

namespace EFG::distribution {
    DistributionFinder::DistributionFinder(const Distribution& distribution, const std::set<categoric::VariablePtr>& containingGroup)
        : evaluator(distribution.evaluator)
        , values(distribution.values) {
        this->indicesContainingGroup.reserve(distribution.getGroup().getVariables().size());
        std::for_each(distribution.getGroup().getVariables().begin(), distribution.getGroup().getVariables().end(), [&](const categoric::VariablePtr& var) {
            auto itVar = containingGroup.find(var);
            if (itVar == containingGroup.end()) {
                throw Error("variable not found");
            }
            this->indicesContainingGroup.push_back(std::distance(containingGroup.begin(), itVar));
        });
    }

    std::pair<const categoric::Combination*, float> DistributionFinder::find(const categoric::Combination& comb) const {
        auto temp = this->findRaw(comb);
        return std::make_pair(temp.first, this->evaluator->evaluate(temp.second));
    }

    std::pair<const categoric::Combination*, float> DistributionFinder::findRaw(const categoric::Combination& comb) const {
        std::vector<std::size_t> combOrdered;
        combOrdered.resize(this->indicesContainingGroup.size());
        for (std::size_t k = 0; k < this->indicesContainingGroup.size(); ++k) {
            combOrdered[k] = comb.data()[this->indicesContainingGroup[k]];
        }

        auto it = this->values->find(categoric::Combination(combOrdered.data(), combOrdered.size()));
        if (it == this->values->end()) return std::make_pair(nullptr, 0.f);
        return std::make_pair(&it->first, it->second);
    }
}
