/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <memory>
#include <optional>
#include <vector>

namespace EFG::train {

using Combinations = std::vector<std::vector<std::size_t>>;

class TrainSet {
public:
  /**
   * @param the set of combinations that will be part of the train set.
   * @throw if the combinations don't have all the same size
   * @throw if the combinations container is empty
   */
  TrainSet(const std::vector<std::vector<std::size_t>> &combinations);

  const auto &getCombinations() const { return *this->combinations; };

  class Iterator;
  Iterator makeIterator() const;

  Iterator makeSubSetIterator(float percentage) const;

private:
  std::shared_ptr<const Combinations> combinations;
};

/**
 * @brief an object able to iterate all the combinations that are part of a
 * training set or a sub portion of it.
 */
class TrainSet::Iterator {
public:
  /**
   * @brief involved train set
   * @param the percentage of combinations to extract from the passed subject.
   * Passing a value equal to 1, means to use all the combinations of the passed
   * subject.
   */
  Iterator(const TrainSet &subject, float percentage);

  template <typename Predicate>
  void forEachSample(const Predicate &pred) const {
    if (combinations_subset.has_value()) {
      const auto &coll = *combinations;
      for (auto index : combinations_subset.value()) {
        pred(coll[index]);
      }
      return;
    }
    for (const auto &sample : *combinations) {
      pred(sample);
    }
  }

  /**
   * @return number of combinations considered by this train set iterator.
   */
  std::size_t size() const;

private:
  std::shared_ptr<const Combinations> combinations;

  std::optional<std::vector<std::size_t>> combinations_subset;
};
} // namespace EFG::train
