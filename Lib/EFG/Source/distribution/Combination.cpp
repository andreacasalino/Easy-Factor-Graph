/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <distribution/Combination.h>
#include <cstring>

namespace EFG::distribution {
    Combination::Combination(const std::vector<std::size_t>& values) 
        : combination(values) {
    }

    Combination::Combination(const Combination& o) 
        : Combination(o.combination) {
    }
}
