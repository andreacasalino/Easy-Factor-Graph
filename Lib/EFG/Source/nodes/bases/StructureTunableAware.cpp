/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/bases/StructureTunableAware.h>
#include <algorithm>

namespace EFG::nodes {
    std::vector<std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>> StructureTunableAware::getFactorsTunable() const {
        std::vector<std::vector<std::shared_ptr<distribution::factor::modif::FactorExponential>>> result;
        result.reserve(this->factorsTunable.size());
        std::for_each(this->factorsTunable.begin(), this->factorsTunable.end(), [&result](const std::set<distribution::factor::modif::FactorExponential*>& s) {
            result.push_back(convert(s));
        });
        return result;
    }
}
