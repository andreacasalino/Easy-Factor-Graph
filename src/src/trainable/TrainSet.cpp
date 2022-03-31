/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/trainable/TrainSet.h>

#include <math.h>

namespace EFG::train {
TrainSet::TrainSet(const std::vector<categoric::Combination> &combinations) {
  if (combinations.empty()) {
    throw Error("empty train set");
  }
  const std::size_t size = combinations.front().size();
  for (const auto &combination : combinations) {
    if ((0 == combination.size()) || (size != combination.size())) {
      throw Error("invalid train set");
    }
  }
  this->combinations =
      std::make_shared<const std::vector<categoric::Combination>>(combinations);
};

TrainSet::Iterator::Iterator(const TrainSet &subject, const float percentage) {
  this->combinations = subject.combinations;
  if (1.f == percentage) {
    return;
  }
  std::size_t numberOfCombinations = 1;
  throw 0; // compute combinations number
           //   numberOfCombinations =
  //       max(static_cast<int>(floor(this->combinations->size() * percentage)),
  //           numberOfCombinations);
  //   numberOfCombinations = min(this->combinations->size(),
  // numberOfCombinations);
  auto &pos = positions.emplace();
  pos.reserve(numberOfCombinations);
  for (std::size_t k = 0; k < numberOfCombinations; ++k) {
    pos.push_back(rand() % this->combinations->size());
  }
}

TrainSet::Iterator TrainSet::makeIterator() const {
  return Iterator{*this, 1.f};
}

TrainSet::Iterator TrainSet::makeSubSetIterator(const float &percentage) const {
  return Iterator{*this, percentage};
}
} // namespace EFG::train
