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

  //////////////////////////////
  // MOVE TO io folder
  //   /**
  //    * @param import the combinations from a textual file where each row
  //    represent
  //    * a combination
  //    * @throw if the file is not readable
  //    * @throw if the parsed combinations don't have all the same size
  //    * @throw if the file is empty
  //    */
  //   explicit TrainSet(const std::string &fileName);

  const std::vector<categoric::Combination> &getCombinations() const {
    return *this->combinations;
  };

  class Iterator;
  Iterator makeIterator() const;

  Iterator makeSubSetIterator(const float &percentage) const;

private:
  std::shared_ptr<const std::vector<categoric::Combination>> combinations;
};

class TrainSet::Iterator {
public:
  /**
   * @return a TrainSet containg some of the combinations stored into this
   * object. The combination to take are randomly decided.
   * @param the percentage of combinations to extract from this object.
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

private:
  std::shared_ptr<const std::vector<categoric::Combination>> combinations;
  std::optional<std::vector<std::size_t>> positions;
};
} // namespace EFG::train
