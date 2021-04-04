/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/Distribution.h>
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

    float Distribution::find(const Combination& comb) const {
        if(comb.size() != this->getGroup().getVariables().size()) {
            throw Error("invalid combination");
        }
        auto it = this->getValuesPtr()->find(comb);
        if(it == this->getValuesPtr()->end()) return 0.f;
        return this->getEvaluatorPtr()->evaluate(it->second);
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

        auto it = this->getValuesPtr()->find(Combination(combOrdered));
        if(it == this->getValuesPtr()->end()) return std::make_pair(nullptr, 0.f);
        return std::make_pair(&it->first , it->second);
    };

 




    // Distribution::Distribution(image::EvaluatorPtr evaluator, const categoric::Group& variables)
    //     : variables(variables) {
    //     this->evaluator = evaluator;
    // }

    // Distribution::Distribution(const categoric::Group& variables)
    //     : Distribution(std::make_shared<image::Basic>(), variables) {
    // }

    // Distribution::Distribution(const Distribution& o) 
    //     : Distribution(o.evaluator->copy() , o.variables) {
    //     this->values = o.values;
    // };

    // Distribution& Distribution::operator=(const Distribution& o) {
    //     this->variables = o.variables;
    //     this->values = o.values;
    //     return *this;
    // }


}
