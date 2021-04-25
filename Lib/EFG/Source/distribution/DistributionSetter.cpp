/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/DistributionSetter.h>
#include <categoric/Range.h>
#include <Error.h>

namespace EFG::distribution {
    void DistributionSetter::setImageRaw(const categoric::Combination& comb, const float& value) {
        this->checkCombination(comb, value);
        auto it = this->values->find(comb);
        if (it == this->values->end()) {
            this->values->emplace(comb, value);
        }
        else {
            it->second = value;
        }
    }

    void DistributionSetter::fillDomain() {
        if (this->values->size() == this->group->size()) {
            return;
        }
        categoric::Range r(this->group->getVariables());
        iterator::forEach(r, [this](const categoric::Range& r) {
            auto it = this->values->find(r.get());
            if (it == this->values->end()) {
                this->values->emplace(r.get(), 0.f);
            }
        });
    }

    void DistributionSetter::setAllImagesRaw(const float& value) {
        if (value < 0.f) {
            throw Error("negative value is not possible");
        }
        this->fillDomain();
        for (auto it = this->values->begin(); it != this->values->end(); ++it) {
            it->second = value;
        }
    }
}
