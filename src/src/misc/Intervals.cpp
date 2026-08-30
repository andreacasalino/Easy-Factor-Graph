/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/misc/Intervals.h>

#include <algorithm>
#include <limits>
#include <optional>

namespace EFG::misc {
Intervals::Intervals(std::vector<IntervalPoint> vals) {
  if (vals.empty() || 0 < vals.front().first) {
    vals.insert(vals.begin(), std::make_pair(0, 0));
  }
  vals.emplace_back(std::make_pair(std::numeric_limits<std::size_t>::max(), 0));
  delimiters = Slot<std::pair<std::size_t, float>>::makeOwning(vals);
}

float Intervals::get(std::size_t index) const {
  auto block = delimiters.get();
  auto it = std::lower_bound(
      block.begin(), block.end(), std::make_pair(index, 0),
      [](const auto &a, const auto &b) { return a.first < b.first; });
  // cannot be delimiters.block.end() as numeric_limist<size_t>::max() is always
  // added as back value in the c'tor
  if (it->first != index) {
    --it;
  }
  return it->second;
}
} // namespace EFG::misc
