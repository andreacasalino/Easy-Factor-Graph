/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/Distribution.h>
#include <distribution/image/Basic.h>
#include <categoric/Range.h>
#include <list>
#include <Error.h>
#include <algorithm>

namespace EFG::distribution {
    bool operator<(const Combination& a, const Combination& b) {
        for(std::size_t k=0; k<a.size(); ++k) {
            if(a.data()[k] < b.data()[k]) return true;
        }
        return false;
    };

    Distribution::Distribution(image::EvaluatorPtr evaluator, const categoric::Group& variables)
        : variables(variables) {
        this->evaluator = evaluator;
    }

    Distribution::Distribution(const categoric::Group& variables)
        : Distribution(std::make_shared<image::Basic>(), variables) {
    }

    Distribution::Distribution(const Distribution& o) 
        : Distribution(o.evaluator->copy() , o.variables) {
        this->values = o.values;
    };

    void Distribution::clear() {
        if(this->isObserved()) {
            throw Error("distribution can't be changed");
        }
        this->values.clear();
    }

    void Distribution::add(const Combination& comb, const float& value) {
        if(value < 0.f) {
            throw Error("negative value is invalid");
        }
        if(this->isObserved()) {
            throw Error("distribution can't be changed");
        }
        if(comb.size() != this->variables.getVariables().size()) {
            throw Error("invalid combination size");
        }
        std::size_t k = 0;
        std::for_each(this->variables.getVariables().begin(), this->variables.getVariables().end(), [&k, &comb](const categoric::VariablePtr& v){
            if(comb.data()[k] >= v->size()) {
                throw Error("combination value exceed variable domain size");
            }
            ++k;
        });
        try {
            this->values.emplace(comb, value);
        }
        catch(...) {
            throw Error("Insertion is not possible");
        }        
    }

    void Distribution::emplaceEntireDomain(const float& value) {
        categoric::Range range(this->variables);
        this->clear();
        iterator::forEach(range, [this, &value](categoric::Range& r){
            this->add(r.getCombination(), value);
        });
    }

    float Distribution::find(const Combination& comb) const {
        if(comb.size() != this->variables.getVariables().size()) {
            throw Error("invalid combination");
        }
        auto it = this->values.find(comb);
        if(it == this->values.end()) return 0.f;
        return this->evaluator->evaluate(it->second);
    }

    std::pair<const Combination*, float> Distribution::find(const Combination& comb, const categoric::Group& group) const {
        std::vector<std::size_t> indices;
        indices.reserve(this->variables.getVariables().size());
        std::for_each(this->variables.getVariables().begin(), this->variables.getVariables().end(), [&indices, &group](const categoric::VariablePtr& v) {
            auto it = group.getVariables().find(v);
            if(it != group.getVariables().end()) {
                throw Error("invalid group");
            }
            indices.push_back(std::distance(group.getVariables().begin(), it));
        });

        std::vector<std::size_t> combOrdered;
        combOrdered.reserve(indices.size());
        for(std::size_t k=0; k<indices.size(); ++k) {
            combOrdered[k] = comb.data()[indices[k]];
        }

        auto it = this->values.find(Combination(combOrdered));
        if(it == this->values.end()) return std::make_pair(nullptr, 0.f);
        return std::make_pair(&it->first , it->second);
    };

    Distribution Distribution::marginalize(const Combination& comb, const categoric::Group& evidences) const {
        if(evidences.size() >= this->variables.getVariables().size()) {
            throw Error("new group should be smaller than initial one");
        }

        std::list<std::size_t> indexRemaining, indexEvidence;
        for(std::size_t k=0; k<this->variables.getVariables().size(); ++k) {
            indexRemaining.push_back(k);
        }
        std::for_each(evidences.getVariables().begin(), evidences.getVariables().end(), [this , &indexRemaining, &indexEvidence](const categoric::VariablePtr& v){
            auto it = this->variables.getVariables().find(v);
            if(it == this->variables.getVariables().end()) {
                throw Error("inexistent variable");
            }
            std::size_t ind = std::distance( this->variables.getVariables().begin(), it);
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

        Distribution marginalized(evidences);
        for(auto it = this->values.begin(); it!=this->values.end(); ++it) {
            if(isMatching(it->first)) {
                marginalized.values.emplace(extractRemainingComb(it->first) , this->evaluator->evaluate(it->second));
            }
        }
        return marginalized;
    }

    categoric::Group mergeGroups(const std::vector<const Distribution*>& distributions){
        if(distributions.size() < 2) {
            throw Error("Two distributions are at least required");
        }
        categoric::Group merged = distributions.front()->getGroup();
        for (std::size_t k=1; k<distributions.size(); ++k) {
            std::for_each(distributions[k]->getGroup().getVariables().begin(), distributions[k]->getGroup().getVariables().end(), [&merged](const categoric::VariablePtr& v) {
                try {   
                    merged.add(v);
                }
                catch(...) {
                }
            });
        }
        return merged;
    };
    Distribution::Distribution(const std::vector<const Distribution*>& distributions) 
        : Distribution(mergeGroups(distributions)) {
        categoric::Range range(this->variables);
        iterator::forEach(range, [this, &distributions](categoric::Range& r){
            float val = 1.f;
            for(auto it = distributions.begin(); it!=distributions.end(); ++it) {
                auto addendum = (*it)->find(r.getCombination(), this->variables);
                if(0.f == addendum.second) {
                    val = 0.f;
                    break;
                }
                val *= addendum.second;
            }
            this->add(r.getCombination(), val);
        }); 
    }
}
