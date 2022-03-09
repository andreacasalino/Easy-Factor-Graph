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
  Variable(const std::size_t &size, const std::string &name);

  inline bool operator==(const Variable &o) const {
    return (this->Size == o.Size) && (this->Name == o.Name);
  };

  inline std::size_t size() const { return this->Size; };
  inline const std::string &name() const { return this->Name; };

protected:
  const size_t Size;
  const std::string Name;
};

using VariablePtr = std::shared_ptr<Variable>;

VariablePtr make_variable(const std::size_t &size, const std::string &name) {
  return std::make_shared<Variable>(size, name);
};
} // namespace EFG::categoric

#include <functional>

namespace std {
template <> struct hash<EFG::categoric::Variable> {
  std::size_t operator()(const EFG::categoric::Variable &v) const {
    (hash<string>()(v.name()) ^ (hash<std::size_t>()(v.size()) << 1));
  }
};

template <> struct hash<EFG::categoric::VariablePtr> {
  std::size_t operator()(const EFG::categoric::VariablePtr &v) const {
    return std::hash<EFG::categoric::Variable>()(*v);
  }
};
} // namespace std
