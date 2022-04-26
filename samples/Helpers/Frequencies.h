/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/GibbsSampler.h>

/// compute the empirical frequencies of a variable (var_sampled), considering
/// a list of realizations (sample) taken as samples from a joint probability
/// distribution (the one involving vars_in_sample)
/// the result is put into marginals
std::vector<float> getEmpiricalMarginalFrequencies(
    categoric::VariablePtr var2Search,
    const std::vector<categoric::Combination> &samples,
    const categoric::VariableSoup &samplesGroup);

/// similar to the above function, computing in single call the empirical
/// distributions of all the variables in vars_to_search
float getEmpiricalFrequencies(
    const categoric::Combination &comb2Search,
    const categoric::VariableSoup &combGroup,
    const std::vector<categoric::Combination> &samples,
    const categoric::VariableSoup &samplesGroup);
