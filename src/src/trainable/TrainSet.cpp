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
TrainSet::TrainSet(const std::vector<std::vector<std::size_t>> &combinations) {
  if (combinations.empty()) {
    throw Error("empty train set");
  }
  const std::size_t size = combinations.front().size();
  for (const auto &combination : combinations) {
    if ((0 == combination.size()) || (size != combination.size())) {
      throw Error("invalid train set");
    }
  }
  this->combinations = std::make_shared<const Combinations>(combinations);
}

TrainSet::Iterator::Iterator(const TrainSet &subject, float percentage) {
  this->combinations = subject.combinations;
  if (1.f == percentage) {
    return;
  }
  if ((percentage <= 0) || (percentage > 1.f)) {
    throw Error::make(percentage,
                      " is an invalid percentage for a TrainSet Iterator");
  }
  int subset_size =
      std::max(0, static_cast<int>(floorf(percentage * combinations->size())));
  auto &subset = combinations_subset.emplace();
  subset.reserve(subset_size);
  for (int k = 0; k < subset_size; ++k) {
    int sampled_pos = rand() % combinations->size();
    subset.push_back(sampled_pos);
  }
}

TrainSet::Iterator TrainSet::makeIterator() const {
  return Iterator{*this, 1.f};
}

TrainSet::Iterator TrainSet::makeSubSetIterator(float percentage) const {
  return Iterator{*this, percentage};
}

std::size_t TrainSet::Iterator::size() const {
  return combinations_subset.has_value() ? combinations_subset->size()
                                         : combinations->size();
}
} // namespace EFG::train
