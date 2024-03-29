/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/factor/Function.h>

#include <ostream>

std::ostream &operator<<(std::ostream &s,
                         const EFG::categoric::VariablesSoup &group);

std::ostream &operator<<(std::ostream &s, const EFG::categoric::Group &group);

std::ostream &operator<<(std::ostream &s, const std::vector<float> &values);

std::ostream &operator<<(std::ostream &s,
                         const std::vector<std::size_t> &values);

std::ostream &operator<<(std::ostream &s,
                         const EFG::factor::Function &distribution);
