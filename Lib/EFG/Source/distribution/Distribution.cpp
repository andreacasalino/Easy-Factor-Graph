/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/Distribution.h>
#include <distribution/image/Basic.h>
#include <categoric/Range.h>
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
        std::size_t k;
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

    Distribution Distribution::collapse(const categoric::Group& group) const {
        if(group.size() >= this->variables.getVariables().size()) {
            throw Error("new group should be smaller than initial one");
        }

        std::set<std::size_t> indices;
        for(std::size_t k=0; k<this->variables.getVariables().size(); ++k) {
            indices.emplace(k);
        }
        std::for_each(group.getVariables().begin(), group.getVariables().end(), [this , &indices](const categoric::VariablePtr& v){
            auto it = this->variables.getVariables().find(v);
            if(it == this->variables.getVariables().end()) {
                throw Error("inexistent variable");
            }
            indices.erase(indices.find(std::distance( this->variables.getVariables().begin(), it)));
        });

        Distribution collapsed(group);
        std::vector<std::size_t> combRaw;
        std::size_t c;
        combRaw.resize(indices.size());
        for(auto it = this->values.begin(); it!=this->values.end(); ++it) {
            c = 0;
            std::for_each(indices.begin(), indices.end(), [&c, &combRaw, &it](const std::size_t& i){
                combRaw[c] = it->first.data()[i];
                ++c;
            });
            collapsed.add(combRaw, this->evaluator->evaluate(it->second));
        }
        return collapsed;
    }
}
