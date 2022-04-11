/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Combination.h>

#include <memory>
#include <optional>

namespace EFG::train {

class TrainSet {
public:
  /**
   * @param the set of combinations that will be part of the train set.
   * @throw if the combinations don't have all the same size
   * @throw if the combinations container is empty
   */
  explicit TrainSet(const std::vector<categoric::Combination> &combinations);

  const std::vector<categoric::Combination> &getCombinations() const {
    return *this->combinations;
  };

  class Iterator;
  Iterator makeIterator() const;

  Iterator makeSubSetIterator(const float &percentage) const;

private:
  std::shared_ptr<const std::vector<categoric::Combination>> combinations;
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
  Iterator(const TrainSet &subject, const float percentage);

  template <typename Predicate>
  void forEachSample(const Predicate &pred) const {
    if (std::nullopt == positions) {
      for (const auto &sample : *combinations) {
        pred(sample);
      }
    } else {
      for (const auto &pos : *positions) {
        pred((*combinations)[pos]);
      }
    }
  }

  /**
   * @return number of combinations considered by this train set iterator.
   */
  std::size_t size() const;

private:
  std::shared_ptr<const std::vector<categoric::Combination>> combinations;
  std::optional<std::vector<std::size_t>> positions;
};
} // namespace EFG::train
