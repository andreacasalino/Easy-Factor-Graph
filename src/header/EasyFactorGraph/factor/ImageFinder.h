/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/Function.h>

namespace EFG::factor {
/**
 * @brief An object used to search for the images associated to
 * sub combinations that are part of a bigger one.
 */
class ImageFinder {
  friend class Immutable;

public:
  /**
   * @brief Searches for matches. For example assume having built this
   * object with a bigger_group equal to <A,B,C,D> while the variables
   * describing the distribution this finder refers to is equal to <B,D>.
   * When passing a comb equal to <0,1,2,0>, this object searches for the
   * immage associated to the sub combination <B,D> = <1,0>.
   * @param the combination of values referring to the bigger_group, which
   * contains the sub combination to search.
   * @return image associated to the passed combination
   */
  float findTransformed(const std::vector<std::size_t> &comb) const {
    return function_->findTransformed(extractSmallerCombination(comb));
  }

  float findImage(const std::vector<std::size_t> &comb) const {
    return function_->findImage(extractSmallerCombination(comb));
  }

private:
  ImageFinder(std::shared_ptr<const Function> function,
              const categoric::VariablesSoup &bigger_group);

  std::shared_ptr<const Function> function_;
  std::vector<std::size_t> indices_in_bigger_group;

  std::vector<std::size_t>
  extractSmallerCombination(const std::vector<std::size_t> &comb) const;
};
} // namespace EFG::factor
