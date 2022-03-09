/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/Group.h>

namespace EFG::categoric {
VariablesSet to_vars_set(const VariablesSoup &soup) {
  VariablesSet result;
  for (const auto &var : soup) {
    result.emplace(var);
  }
  return result;
}

Group::Group(const VariablesSoup &group)
    : group(group), group_sorted(to_vars_set(group)) {
  if (group.empty()) {
    throw Error{"Variables group can't be empty"};
  }
  if (group.size() != group_sorted.size()) {
    throw Error{"Variables group with multiple variables with same name"};
  }
}

Group::Group(const VariablePtr &var) {
  group.push_back(var);
  group_sorted.emplace(var);
}

void Group::add(const VariablePtr &var) {
  auto it = group_sorted.find(var);
  if (it == group_sorted.end()) {
    group.push_back(var);
    group_sorted.emplace(var);
    return;
  }
  throw Error{var->name(), ", already existing inside group of variables"};
}

void Group::replaceVariables(const VariablesSoup &new_variables) {
  auto new_group_sorted = to_vars_set(new_variables);
  if (new_group_sorted.size() != group.size()) {
    throw Error{"Invalid new variables group"};
  }
  for (std::size_t k = 0; k < new_variables.size(); ++k) {
    if (new_variables[k]->size() < group[k]->size()) {
      throw Error{"Invalid new variables group"};
    }
  }
  group = new_variables;
  group_sorted = std::move(new_group_sorted);
}

std::size_t Group::size() const {
  std::size_t S = 1;
  for (const auto &var : group) {
    S *= var->size();
  }
  return S;
}

VariablesSet &operator-=(VariablesSet &subject, const VariablesSet &to_remove) {
  for (const auto &rem : to_remove) {
    auto it = subject.find(rem);
    if (it != subject.end()) {
      subject.erase(it);
    }
  }
  return subject;
}

VariablesSet get_complementary(const VariablesSet &entire_set,
                               const VariablesSet &subset) {
  VariablesSet result = entire_set;
  result -= subset;
  return result;
};
} // namespace EFG::categoric
