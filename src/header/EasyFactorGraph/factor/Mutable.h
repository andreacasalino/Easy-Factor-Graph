/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Function.h>

namespace EFG::factor {
class Mutable {
public:
  Mutable(const Mutable &o) = delete;
  Mutable &operator=(const Mutable &) = delete;
  Mutable(Mutable &&o) = delete;
  Mutable &operator=(Mutable &&) = delete;

  /**
   * @brief sets the raw value of the image related to the passed combination.
   * In case the combination is currently not part of the distribution, it is
   * added to the combinations map, with the passed raw image value.
   * @param the combination whose raw image must be set
   * @param the raw image value to assume
   * @throw passing a negative number for value
   */
  void set(const std::vector<std::size_t> &comb, float value);

  /**
   * @brief Removes all the combinations from the combinations map.
   */
  void clear() { function_->clear(); };

  /**
   * @brief Replaces the variables this distribution should refer to.
   */
  void replaceVariables(const categoric::VariablesSoup &new_variables) {
    function_->vars().replaceVariables(new_variables);
  };

protected:
  Mutable(FunctionPtr data);

  auto &functionMutable() { return *function_; }

private:
  FunctionPtr function_;
};
} // namespace EFG::factor
