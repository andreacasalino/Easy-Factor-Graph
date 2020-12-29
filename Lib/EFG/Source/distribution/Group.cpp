#include <distribution/Group.h>

namespace EFG::distr {

    Group::Group(const std::vector<CategoricVariable*>& group) : group(group) {

        if (!CategoricVariable::AreAllVarsDifferent(group)) throw std::runtime_error("repeated variable in set");

        std::size_t K = group.size();
        this->groupObservers.reserve(K);
        for (size_t k = 0; k < K; ++k) this->groupObservers.emplace_back(*this->group[k]);

    }

    void Group::replace(const std::vector<CategoricVariable*>& newGroup) {

        std::size_t K = newGroup.size();
        if (K != this->group.size()) throw std::runtime_error("the new domain has not the same caridinality of the previous");
        if (!CategoricVariable::AreAllVarsDifferent(newGroup)) throw std::runtime_error("repeated variable in set");

        for (size_t k = 0; k < K; ++k) {
            if (newGroup[k]->size() != this->group[k]->size()) throw std::runtime_error("the new domain does not match for all the variables the sizes");
        }

        this->groupObservers.clear();
        this->group = newGroup;
        for (size_t k = 0; k < K; ++k) this->groupObservers.emplace_back(*this->group[k]);

    }

}
