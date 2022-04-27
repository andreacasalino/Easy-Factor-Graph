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
                      const std::vector<EFG::categoric::Combination> &samples,
                      const EFG::categoric::VariablesSoup &samplesGroup);

float getEmpiricalProbability(
    const EFG::categoric::Combination &comb2Search,
    const EFG::categoric::VariablesSoup &combGroup,
    const std::vector<EFG::categoric::Combination> &samples,
    const EFG::categoric::VariablesSoup &samplesGroup);

std::vector<float> make_distribution(const std::vector<float> &values);
