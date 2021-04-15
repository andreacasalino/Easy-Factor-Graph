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
        this->indices.reserve(distribution.getGroup().getVariables().size());
        std::for_each(distribution.getGroup().getVariables().begin(), distribution.getGroup().getVariables().end(), [&](const categoric::VariablePtr& var) {
            auto itVar = containingGroup.find(var);
            if (itVar == containingGroup.end()) {
                throw Error("variable not found");
            }
            this->indices.push_back(std::distance(containingGroup.begin(), itVar));
        });
    }

    std::pair<const Combination*, float> DistributionFinder::find(const Combination& comb) const {
        std::vector<std::size_t> combOrdered;
        combOrdered.resize(indices.size());
        for (std::size_t k = 0; k < indices.size(); ++k) {
            combOrdered[k] = comb.data()[indices[k]];
        }

        auto it = this->values->find(Combination(combOrdered));
        if (it == this->values->end()) return std::make_pair(nullptr, 0.f);
        return std::make_pair(&it->first, it->second);
    }

    std::pair<const Combination*, float> DistributionFinder::findRaw(const Combination& comb) const {
        auto temp = this->find(comb);
        return std::make_pair(temp.first, this->evaluator->evaluate(temp.second));
    }
}
