/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/modifiers/Changer.h>
#include <categoric/Range.h>

namespace EFG::distribution {
    std::map<categoric::Combination, float>::const_iterator Changer::add(const categoric::Combination& comb, const float& value) {
        this->checkCombination(comb, value);
        auto res = this->values->emplace(comb, value);
        return res.first;
    }

    void Changer::setImageEntireDomain(const float& value) {
        categoric::Range range(this->getGroup().getVariables());
        this->clear();
        iterator::forEach(range, [this, &value](categoric::Range& r) {
            this->values->emplace(r.get(), value);
        });
    }

    void Changer::emplaceEntireDomain() {
        if (this->group->size() == this->values->size()) {
            return;
        }
        categoric::Range range(this->getGroup().getVariables());
        iterator::forEach(range, [this](categoric::Range& r) {
            auto it = this->values->find(r.get());
            if (it == this->values->end()) {
                this->values->emplace(r.get(), 0.f);
            }
        });
    }
}
