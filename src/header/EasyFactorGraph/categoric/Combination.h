/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <vector>

namespace EFG::categoric {
/**
 * @brief An immutable combination of discrete values
 */
class Combination {
public:
  /**
   * @brief A buffer of zeros with the passed size is created
   * @param the size of the combination to build
   * @throw if bufferSize is 0
   */
  Combination(const std::size_t bufferSize);

  /**
   * @param the values that will characterize the Combination
   * @throw if buffer is empty
   */
  Combination(std::vector<std::size_t> &&buffer);

  Combination(const Combination &o);

  /**
   * @brief compare two equally sized combination.
   * Examples of ordering:
   * <0,0,0> < <0,1,0>
   * <0,1> < <1,0>
   * @throw when the passed combination has a different size
   */
  bool operator<(const Combination &o) const;

  std::size_t size() const { return values.size(); };
  const std::vector<std::size_t> &data() const { return values; };

private:
  const std::vector<std::size_t> values;
};

bool operator==(const Combination &a, const Combination &b);

bool operator!=(const Combination &a, const Combination &b);
} // namespace EFG::categoric
