/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/Samples.h>
#include <EasyFactorGraph/structure/Structure.h>

std::vector<float> getEmpiricalMarginals(std::size_t var_index,
                                         EFG::categoric::VarStateSize var_size,
                                         const EFG::misc::Samples &samples);

float getEmpiricalProbability(
    const std::vector<
        std::pair<std::size_t /* var index */,
                  EFG::categoric::VarStateSize /* var value to seach */>>
        &to_search,
    const EFG::misc::Samples &samples);

float getEmpiricalProbabilityInsideHidden(
    const std::vector<
        std::pair<std::size_t /* var index ... absolute */,
                  EFG::categoric::VarStateSize /* var value to seach */>>
        &to_search,
    const EFG::misc::Samples &samples,
    const EFG::structure::Structure &context);

std::vector<float> make_distribution(const std::vector<float> &values);
