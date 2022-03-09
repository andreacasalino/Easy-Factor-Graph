/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Variable.h>
#include <unordered_set>

namespace EFG::categoric {
using Variables = std::unordered_set<VariablePtr>;

/**
 * @brief An ensemble of categoric variables. Each variable in the ensemble
 * should have its own unique name.
 */
class Group {
public:
  /**
   * @param the initial variables of the group
   */
  explicit Group(const Variables &group);

  /**
   * @param the initial variable to put in the group
   */
  explicit Group(const VariablePtr &var);

  /**
   * @param the first initial variable to put in the group
   * @param the second initial variable to put in the group
   * @param all the other initial variables
   */
  template <typename... Vars>
  Group(const VariablePtr &varA, const VariablePtr &varB, Vars... vars) {
    this->add(varA);
    this->add(varB, vars...);
  }

  /**
   * @throw In case of size mismatch with the previous variables set:
   * the sizes of the 2 groups should be the same and the elements in
   * the same positions must have the same domain size.
   */
  /**
   * @brief replaces the group of variables.
   * @throw In case of size mismatch with the previous variables set:
   * the sizes of the 2 groups should be the same and the elements in
   * the same positions must have the same domain size
   */
  void replaceVariables(const Group &o);

  inline bool operator==(const Group &o) const {
    return (this->group == o.group);
  };

  /**
   * @param the variable to add in the group
   * @throw in case a variable with the same name is already part of the group
   */
  void addVariable(VariablePtr var);

  template <typename... Vars> void addVariables(VariablePtr var, Vars... vars) {
    this->add(var);
    this->add(vars...);
  }

  /** @return the size of the joint domain of the group.
   * For example the group <A,B,C> with sizes <2,4,3> will have a joint domain
   * of size 2x4x3 = 24
   */
  std::size_t size() const;

  inline const Variables &getVariables() const { return this->group; }

protected:
  Variables group;
};

/**
 * @brief get the complementary group of the subset w.r.t a certain set.
 * For instance the complementary of <A,C> w.r.t. <A,B,C,D,E> is <B,D,E>
 * @throw when not all variables in subset are present in set
 * @throw when the complementary is an empty set
 */
Variables get_complementary(const Variables &entire_set,
                            const Variables &subset);
} // namespace EFG::categoric
