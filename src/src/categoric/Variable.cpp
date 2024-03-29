/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/Variable.h>

namespace EFG::categoric {
Variable::Variable(std::size_t size, const std::string &name)
    : size_(size), name_(name) {
  if (name.size() == 0)
    throw Error("Empty name for Variable forbidden");
  if (size == 0)
    throw Error("Null size for Variable forbidden");
}

VariablePtr make_variable(std::size_t size, const std::string &name) {
  return std::make_shared<Variable>(size, name);
}
} // namespace EFG::categoric
