/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/Distribution.h>
#include <distribution/DistributionIterator.h>
#include <distribution/DistributionFinder.h>
#include <categoric/Range.h>
#include <Error.h>
#include <algorithm>

namespace EFG::distribution {
    void Distribution::checkCombination(const Combination& comb, const float& value) const {
        if (value < 0.f) {
            throw Error("negative value is invalid");
        }
        if (comb.size() != this->getGroup().getVariables().size()) {
            throw Error("invalid combination size");
        }
        std::size_t k = 0;
        std::for_each(this->getGroup().getVariables().begin(), this->getGroup().getVariables().end(), [&k, &comb](const categoric::VariablePtr& v) {
            if (comb.data()[k] >= v->size()) {
                throw Error("combination value exceed variable domain size");
            }
            ++k;
            });
    }

    float Distribution::find(const Combination& comb) const {
        return this->evaluator->evaluate(this->findRaw(comb));
    }

    float Distribution::findRaw(const Combination& comb) const {
        if (comb.size() != this->getGroup().getVariables().size()) {
            throw Error("invalid combination");
        }
        auto it = this->values->find(comb);
        if (it == this->values->end()) return 0.f;
        return it->second;
    }

    DistributionIterator Distribution::getIterator() const {
        return DistributionIterator(*this);
    }

    DistributionFinder Distribution::getFinder(const std::set<categoric::VariablePtr>& containingGroup) const {
        return DistributionFinder(*this, containingGroup);
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
                auto it = this->values->find(Combination(jointDomain.get()));
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
