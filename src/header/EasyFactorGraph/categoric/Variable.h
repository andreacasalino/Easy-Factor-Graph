/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <memory>
#include <string>

namespace EFG::categoric {
/**
 * @brief An object representing an immutable categoric variable.
 */
class Variable {
public:
  /**
   * @param domain size of this variable
   * @param name used to label this varaible.
   * @throw passing 0 as size
   * @throw passing an empty string as name
   */
  Variable(std::size_t size, const std::string &name);

  std::size_t size() const { return this->size_; };
  const std::string &name() const { return this->name_; };

  bool operator==(const Variable &o) const {
    return (this->name_ == o.name_) && (this->size_ == o.size_);
  }

protected:
  const size_t size_;
  const std::string name_;
};

using VariablePtr = std::shared_ptr<Variable>;

VariablePtr make_variable(std::size_t size, const std::string &name);
} // namespace EFG::categoric

namespace std {
template <> struct hash<EFG::categoric::Variable> {
  std::size_t operator()(const EFG::categoric::Variable &subject) const {
    return std::hash<std::string>{}(subject.name());
  }
};
} // namespace std
