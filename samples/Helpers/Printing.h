/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/GroupRange.h>
#include <EasyFactorGraph/distribution/Distribution.h>

#include <ostream>

std::ostream &operator<<(std::ostream &s,
                         const EFG::categoric::VariablesSoup &group);

std::ostream &operator<<(std::ostream &s, const EFG::categoric::Group &group);

void print_range(std::ostream &s, const EFG::categoric::Group &group);

std::ostream &operator<<(std::ostream &s,
                         const EFG::distribution::Distribution &distribution);
