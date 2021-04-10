/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/Distribution.h>
#include <distribution/DistributionIterator.h>
#include <categoric/Range.h>
#include <Error.h>
#include <algorithm>

namespace EFG::distribution {
    float Distribution::find(const Combination& comb) const {
        if(comb.size() != this->getGroup().getVariables().size()) {
            throw Error("invalid combination");
        }
        auto it = this->values->find(comb);
        if(it == this->values->end()) return 0.f;
        return this->evaluator->evaluate(it->second);
    }

    std::pair<const Combination*, float> Distribution::find(const Combination& comb, const categoric::Group& group) const {
        std::vector<std::size_t> indices;
        indices.reserve(this->getGroup().getVariables().size());
        std::for_each(this->getGroup().getVariables().begin(), this->getGroup().getVariables().end(), [&indices, &group](const categoric::VariablePtr& v) {
            auto it = group.getVariables().find(v);
            if(it == group.getVariables().end()) {
                throw Error("invalid group");
            }
            indices.push_back(std::distance(group.getVariables().begin(), it));
        });

        std::vector<std::size_t> combOrdered;
        combOrdered.resize(indices.size());
        for(std::size_t k=0; k<indices.size(); ++k) {
            combOrdered[k] = comb.data()[indices[k]];
        }

        auto it = this->values->find(Combination(combOrdered));
        if(it == this->values->end()) return std::make_pair(nullptr, 0.f);
        return std::make_pair(&it->first , it->second);
    };

    DistributionIterator Distribution::getIterator() const {
        return DistributionIterator(*this);
    }

    std::vector<float> Distribution::getProbabilities() const {
        std::vector<float> probs;
        probs.reserve(this->group->size());
        if (this->group->size() == this->values->size()) {
            for (auto it = this->values->begin(); it != this->values->end(); ++it) {
                probs.push_back(this->evaluator->evaluate(it->second));
            }
        }
        else {
            categoric::Range jointDomain(*this->group);
            iterator::forEach(jointDomain, [this, &probs](categoric::Range& jointDomain) {
                auto it = this->values->find(jointDomain.get());
                if (it == this->values->end()) {
                    probs.push_back(0.f);
                }
                else {
                    probs.push_back(this->evaluator->evaluate(it->second));
                }
            });
        }
        // normalize values
        float sum = 0.f;
        std::for_each(probs.begin(), probs.end(), [&sum](const float& v) { sum += v; });
        std::for_each(probs.begin(), probs.end(), [&sum](float& v) { v /= sum; });
        return probs;
    }
}
