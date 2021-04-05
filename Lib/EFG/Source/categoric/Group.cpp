/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <categoric/Group.h>
#include <algorithm>
#include <Error.h>

namespace EFG::categoric {
    Group::Group(VariablePtr var) {
        this->add(var);
    }

    Group::Group(VariablePtr varA, VariablePtr varB) {
        this->add(varA);
        this->add(varB);
    }

    void Group::add(VariablePtr var) {
        if(this->group.end() != this->group.find(var)) {
            throw Error("Group", "A variable with the same name is already part of the group");
        }
        this->group.emplace(var);
    }

    Group& Group::operator=(const Group& replacer) {
        if(replacer.group.size() != this->group.size()) {
            throw Error("replacing variables should be in number the same");
        }
        auto itThis = this->group.begin();
        std::for_each(replacer.group.begin(), replacer.group.end(), [&itThis](const VariablePtr& v){
            if((*itThis)->size() != v->size()) {
                throw Error("replacing variables should have the same sizes");
            }
            ++itThis;
        });
        this->group = replacer.group;
        return *this;
    }

    std::size_t Group::size() const {
        std::size_t S = 0;
        std::for_each(this->group.begin(), this->group.end(), [&S](const VariablePtr& v){
            S += v->size();
        });
        return S;
    }
}
