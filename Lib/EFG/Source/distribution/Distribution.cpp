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
            if(a.data()[k] != b.data()[k]) {
                return (a.data()[k] < b.data()[k]); 
            }
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

    Distribution& Distribution::operator=(const Distribution& o) {
        this->variables = o.variables;
        this->values = o.values;
        return *this;
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
    Distribution::Distribution(const std::set<const Distribution*>& distr)
        : Distribution(mergeGroups(distr)) {
        categoric::Range jointDomain(this->variables);
        iterator::forEach(jointDomain, [this, &distr](const categoric::Range& jointDomain) {
            float val = 1.f;
            Combination comb(jointDomain.get());
            for (auto it = distr.begin(); it != distr.end(); ++it) {
                auto result = (*it)->find(comb, this->variables);
                if (nullptr == result.first) {
                    val = 0.f;
                    break;
                }
                val *= result.second;
            }
            if (0.f != val) {
                this->add(comb, val);
            }
        });
    }

    ///////////////////// modifiers /////////////////////

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

    void Distribution::setImageEntireDomain(const float& value) {
        categoric::Range range(this->variables);
        this->clear();
        iterator::forEach(range, [this, &value](categoric::Range& r){
            this->add(Combination(r.get()), value);
        });
    }

    void Distribution::emplaceEntireDomain() {
        categoric::Range range(this->variables);
        iterator::forEach(range, [this](categoric::Range& r){
            Combination comb(r.get());
            auto it = this->values.find(comb);
            if(it == this->values.end()) {
                this->values.emplace(comb, 0.f);
            }
        });
    }

    /////////////////////   query   /////////////////////

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

        auto it = this->values.find(Combination(combOrdered));
        if(it == this->values.end()) return std::make_pair(nullptr, 0.f);
        return std::make_pair(&it->first , it->second);
    };

    Distribution Distribution::marginalize(const Combination& comb, const categoric::Group& evidences) const {
        if(evidences.getVariables().size() >= this->variables.getVariables().size()) {
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

}
