/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <CombinationMaker.h>

namespace EFG::sample {
    categoric::Combination makeCombination(const std::vector<std::size_t>& values) {
        return categoric::Combination(values.data(), values.size());
    };
}
