/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/modifiers/Changer.h>
#include <categoric/Range.h>

namespace EFG::distribution {
    void Changer::clear() {
        this->checkModifiable();
        this->values->clear();
    }

    std::map<Combination, float>::const_iterator Changer::add(const Combination& comb, const float& value) {
        this->checkModifiable();
        this->checkCombination(comb, value);
        auto res = this->values->emplace(comb, value);
        return res.first;
    }

    void Changer::setImageEntireDomain(const float& value) {
        this->checkModifiable();
        categoric::Range range(this->getGroup());
        this->clear();
        iterator::forEach(range, [this, &value](categoric::Range& r) {
            this->values->emplace(Combination(r.get()), value);
        });
    }

    void Changer::emplaceEntireDomain() {
        this->checkModifiable();
        if (this->group->size() == this->values->size()) {
            return;
        }
        categoric::Range range(this->getGroup());
        iterator::forEach(range, [this](categoric::Range& r) {
            Combination comb(r.get());
            auto it = this->values->find(comb);
            if (it == this->values->end()) {
                this->values->emplace(comb, 0.f);
            }
        });
    }
}
