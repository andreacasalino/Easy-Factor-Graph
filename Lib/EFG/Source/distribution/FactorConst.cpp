/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/FactorConst.h>
#include <distribution/image/Basic.h>
#include <categoric/Range.h>
#include <distribution/DistributionIterator.h>
#include <Error.h>
#include <algorithm>
#include <list>

namespace EFG::distribution {
    FactorConst::FactorConst(const categoric::Group& group)
        : DistributionBase(group, std::make_unique<image::Basic>()) {
    }

    categoric::Group mergeGroups(const std::set<const Distribution*>& distr) {
        if (distr.size() <= 1) {
            throw Error("you can merge at least 2 distributions");
        }
        auto it = distr.begin();
        categoric::Group merged((*it)->getGroup());
        ++it;
        std::for_each(it, distr.end(), [&merged](const Distribution* d) {
            for (auto itV = d->getGroup().getVariables().begin(); itV!= d->getGroup().getVariables().end(); ++itV) {
                try {
                    merged.add(*itV);
                }
                catch (...) {
                }
            }
        });
        return merged;
    }
    FactorConst::FactorConst(const std::set<const Distribution*>& distr)
        : FactorConst(mergeGroups(distr)) {
        categoric::Range jointDomain(this->getGroup());
        iterator::forEach(jointDomain, [this, &distr](const categoric::Range& jointDomain) {
            float val = 1.f;
            Combination comb(jointDomain.get());
            for (auto it = distr.begin(); it != distr.end(); ++it) {
                auto result = (*it)->find(comb, this->getGroup());
                if (nullptr == result.first) {
                    val = 0.f;
                    break;
                }
                val *= result.second;
            }
            if (0.f != val) {
                this->values->emplace(comb, val);
            }
        });
    }

    FactorConst::FactorConst(const DistributionBase& toMarginalize, const Combination& comb, const categoric::Group& evidences)
        : FactorConst(evidences) {
        if(evidences.getVariables().size() >= toMarginalize.getGroup().getVariables().size()) {
            throw Error("new group should be smaller than initial one");
        }

        std::list<std::size_t> indexRemaining, indexEvidence;
        for(std::size_t k=0; k<toMarginalize.getGroup().getVariables().size(); ++k) {
            indexRemaining.push_back(k);
        }
        std::for_each(evidences.getVariables().begin(), evidences.getVariables().end(), [&toMarginalize , &indexRemaining, &indexEvidence](const categoric::VariablePtr& v){
            auto it = toMarginalize.getGroup().getVariables().find(v);
            if(it == toMarginalize.getGroup().getVariables().end()) {
                throw Error("inexistent variable");
            }
            std::size_t ind = std::distance( toMarginalize.getGroup().getVariables().begin(), it);
            indexEvidence.push_back(ind);
            indexRemaining.remove(ind);
        });

        auto isMatching = [&indexEvidence, &comb](const Combination& combThis){
            std::size_t k=0;
            for (auto it = indexEvidence.begin(); it!=indexEvidence.end(); ++it) {
                if(combThis.data()[*it] != comb.data()[k]) {
                    return false;
                }
                ++k;
            }
            return true;
        };

        auto extractRemainingComb = [&indexRemaining](const Combination& combThis){
            std::vector<std::size_t> combRaw;
            combRaw.reserve(indexRemaining.size());
            for (auto it = indexRemaining.begin(); it!=indexRemaining.end(); ++it) {
                combRaw.push_back(combThis.data()[*it]);
            }
            return Combination(combRaw);
        };

        auto it = toMarginalize.getIterator();
        iterator::forEach(it, [this](const DistributionIterator& it){
            this->values->emplace(extractRemainingComb(it.getCombination()), it.getImage());
        });
    }
}
