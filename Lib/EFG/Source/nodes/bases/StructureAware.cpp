/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <nodes/bases/StructureAware.h>
#include <algorithm>

namespace EFG::nodes {
    template <typename F>
    std::vector<std::shared_ptr<F>> convert(const std::set<F*>& collection) {
        std::vector<std::shared_ptr<F>> result;
        result.reserve(collection.size());
        std::for_each(collection.begin(), collection.end(), [&result](F* f) {
            result.push_back(convert(f));
        });
        return result;
    };

    std::vector<std::shared_ptr<distribution::factor::cnst::Factor>> StructureAware::getFactors() const {
        return convert(this->factors);
    }

    std::vector<std::shared_ptr<distribution::factor::cnst::FactorExponential>> StructureAware::getFactorsExp() const {
        return convert(this->factorsExp);
    }
}
