/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/GibbsSampler.h>

std::vector<float>
getEmpiricalMarginals(EFG::categoric::VariablePtr var2Search,
                      const std::vector<std::vector<std::size_t>> &samples,
                      const EFG::categoric::VariablesSoup &samplesGroup);

float getEmpiricalProbability(
    const std::vector<std::size_t> &comb2Search,
    const EFG::categoric::VariablesSoup &combGroup,
    const std::vector<std::vector<std::size_t>> &samples,
    const EFG::categoric::VariablesSoup &samplesGroup);

std::vector<float> make_distribution(const std::vector<float> &values);
