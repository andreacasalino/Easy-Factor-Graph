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
  if ((percentage <= 0) || (percentage > 1.f)) {
    throw Error{percentage,
                " is an invalid percentage for a TrainSet Iterator"};
  }
  int subset_size =
      std::max(0, static_cast<int>(floorf(percentage * combinations->size())));
  auto &subset = combinations_subset.emplace();
  subset.reserve(subset_size);
  for (int k = 0; k < subset_size; ++k) {
    int sampled_pos = rand() % combinations->size();
    auto to_add = combinations->begin();
    std::advance(to_add, sampled_pos);
    subset.push_back(to_add);
  }
}

TrainSet::Iterator TrainSet::makeIterator() const {
  return Iterator{*this, 1.f};
}

TrainSet::Iterator TrainSet::makeSubSetIterator(const float &percentage) const {
  return Iterator{*this, percentage};
}

std::size_t TrainSet::Iterator::size() const {
  if (std::nullopt == combinations_subset) {
    return combinations->size();
  }
  return combinations_subset->size();
}
} // namespace EFG::train
