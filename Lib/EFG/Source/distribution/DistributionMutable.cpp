/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/DistributionMutable.h>
#include <categoric/Range.h>
#include <Error.h>
#include <algorithm>

namespace EFG::distribution {
    void DistributionSettable::checkCombination(const Combination& comb, const float& value) const {
        if(value < 0.f) {
            throw Error("negative value is invalid");
        }
        if(comb.size() != this->getGroup().getVariables().size()) {
            throw Error("invalid combination size");
        }
        std::size_t k = 0;
        std::for_each(this->getGroup().getVariables().begin(), this->getGroup().getVariables().end(), [&k, &comb](const categoric::VariablePtr& v){
            if(comb.data()[k] >= v->size()) {
                throw Error("combination value exceed variable domain size");
            }
            ++k;
        });
    }

    void DistributionSettable::checkModifiable() const {
        if(this->isObserved()) {
            throw Error("distribution can't be changed");
        }
    }

    void DistributionSettable::replaceGroup(const categoric::Group& newGroup) {
        this->checkModifiable();
        *this->getGroupPtr() = newGroup;
    }

    void DistributionSettable::set(const Combination& comb, const float& value) {
        this->checkModifiable();
        this->checkCombination(comb, value);
        auto it = this->getValuesPtr()->find(comb);
        if(it == this->getValuesPtr()->end()) {
            // add the combination and set it
            auto emplaced =  this->getValuesPtr()->emplace(comb, value);
            emplaced.first->second = value;
        }
        it->second = value;
    }



    void DistributionModifiable::clear() {
        this->checkModifiable();
        this->getValuesPtr()->clear();
    }

    std::map<Combination, float>::const_iterator DistributionModifiable::add(const Combination& comb, const float& value) {
        this->checkModifiable();
        this->checkCombination(comb, value);
        auto res = this->getValuesPtr()->emplace(comb, value);
        return res.first;
    }

    void DistributionModifiable::setImageEntireDomain(const float& value) {
        this->checkModifiable();
        categoric::Range range(this->getGroup());
        this->clear();
        iterator::forEach(range, [this, &value](categoric::Range& r){
            this->getValuesPtr()->emplace(Combination(r.get()), value);
        });
    }

    void DistributionModifiable::emplaceEntireDomain() {
        this->checkModifiable();
        categoric::Range range(this->getGroup());
        iterator::forEach(range, [this](categoric::Range& r){
            Combination comb(r.get());
            auto it = this->getValuesPtr()->find(comb);
            if(it == this->getValuesPtr()->end()) {
                this->getValuesPtr()->emplace(comb, 0.f);
            }
        });
    }
}
