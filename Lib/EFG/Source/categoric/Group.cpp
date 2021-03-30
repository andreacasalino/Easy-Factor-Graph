/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <categoric/Group.h>
#include <Error.h>
#include <algorithm>

namespace EFG::categoric {
    inline bool operator<(const VariablePtr& a, const VariablePtr& b) {
        return (a->name() < b->name());
    };

    Group::Group(VariablePtr varA, VariablePtr varB) {
        this->group.emplace(varA);
        this->group.emplace(varB);
    }

    void Group::add(VariablePtr var) {
        if(this->group.end() != this->group.find(var)) {
            throw Error("Group", "A variable with the same name is already part of the group");
        }
        this->group.emplace(var);
    }

    std::size_t Group::size() const {
        std::size_t S = 0;
        std::for_each(this->group.begin(), this->group.end(), [&S](const VariablePtr& v){
            S += v->size();
        });
        return S;
    }
}
