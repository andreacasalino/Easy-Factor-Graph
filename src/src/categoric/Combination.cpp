/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/Combination.h>

namespace EFG::categoric {
namespace {
std::vector<std::size_t> zeros(const std::size_t bufferSize) {
  std::vector<std::size_t> result;
  result.reserve(bufferSize);
  for (std::size_t k = 0; k < bufferSize; ++k) {
    result.push_back(0);
  }
  return result;
}
} // namespace

Combination::Combination(const std::size_t bufferSize)
    : Combination(zeros(bufferSize)) {}

Combination::Combination(std::vector<std::size_t> &&buffer)
    : values(std::move(buffer)) {
  if (0 == values.size()) {
    throw Error("Size of Combination should be at least 1");
  }
}

bool Combination::operator<(const Combination &o) const {
  if (this->values.size() != o.values.size()) {
    throw Error("Uncomparable Combinations");
  }
  const auto size = this->values.size();
  for (std::size_t k = 0; k < size; ++k) {
    if (this->values[k] != o.values[k]) {
      return (this->values[k] < o.values[k]);
    }
  }
  return false;
};
} // namespace EFG::categoric
