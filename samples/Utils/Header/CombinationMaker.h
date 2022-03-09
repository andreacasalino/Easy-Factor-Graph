/**
 * Author:    Andrea Casalino
 * Created:   03.01.2020
 *
 * report any bug to andrecasa91@gmail.com.
 **/


#ifndef EFG_SAMPLE_COMBINATION_MAKER_H
#define EFG_SAMPLE_COMBINATION_MAKER_H

#include <categoric/Combination.h>

namespace EFG::sample {
    categoric::Combination makeCombination(const std::vector<std::size_t>& values);
}

#endif
