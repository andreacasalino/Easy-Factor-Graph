/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/structure/SpecialFactors.h>
#include <EasyFactorGraph/structure/Types.h>

namespace EFG::strct {

class StateAware {
public:
  virtual ~StateAware() = default;

  /**
   * @return all the variables that are part of the model.
   */
  const auto &getAllVariables() const { return state_.variables; }

  /**
   * @return all the variables defining the hidden set of variables
   */
  categoric::VariablesSet getHiddenVariables() const;
  /**
   * @return all the variables defining the evidence set
   */
  categoric::VariablesSet getObservedVariables() const;

  /**
   * @return all the variables defining the evidence set, together with the
   * associated values
   */
  const auto &getEvidences() const { return state_.evidences; };

  /**
   * @return the variable in the model with the passed name
   * @throw in case no variable with the specified name exists in this model
   */
  categoric::VariablePtr findVariable(const std::string &name) const;

  StateAware(const StateAware &) = delete;
  StateAware &operator=(const StateAware &) = delete;
  StateAware(StateAware &&) = delete;
  StateAware &operator=(StateAware &&) = delete;

protected:
  struct NodeLocation {
    Node *node;
    std::variant<HiddenClusters::iterator, Evidences::iterator> location;
  };

  struct GraphState {
    categoric::VariablesSoup variables;
    Nodes nodes;
    HiddenClusters clusters;
    Evidences evidences;
  };

  StateAware() = default;

  const GraphState &state() const { return state_; }
  GraphState &stateMutable() { return state_; }

  std::optional<NodeLocation> locate(const categoric::VariablePtr &var) const;
  std::optional<NodeLocation> locate(const std::string &var_name) const {
    return locate(findVariable(var_name));
  }

private:
  mutable GraphState state_;
};
} // namespace EFG::strct
