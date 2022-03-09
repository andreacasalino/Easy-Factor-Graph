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
   * @throw if bufferSize is 0
   */
  Combination(std::size_t bufferSize);

  Combination(std::vector<std::size_t> &&buffer);

  /**
   * @brief compare two equally sized combination.
   * Examples of ordering:
   * <0,0,0> < <0,1,0>
   * <0,1> < <1,0>
   * @throw when o has a different size
   */
  bool operator<(const Combination &o) const;

  inline std::size_t size() const { return values.size(); };
  inline const std::vector<std::size_t> &data() const { return values; };

private:
  const std::vector<std::size_t> values;
};

bool operator==(const Combination &a, const Combination &b) {
  return a.data() == b.data();
};

bool operator!=(const Combination &a, const Combination &b) {
  return a.data() != b.data();
};
} // namespace EFG::categoric
