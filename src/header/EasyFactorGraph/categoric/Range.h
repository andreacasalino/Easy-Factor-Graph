/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/categoric/Types.h>

#include <array>
#include <optional>
#include <vector>

namespace EFG::categoric {
/**
 * @brief This object allows to iterate all the elements in the joint domain
 * of a group of variables, without precomputing all the elements in such
 * domain. For example when having a domain made by variables = { A (size = 2),
 * B (size = 3), C (size  = 2)  }, the elements in the joint domain that will be
 * iterated are:
 * <0,0,0>
 * <0,0,1>
 * <0,1,0>
 * <0,1,1>
 * <0,2,0>
 * <0,2,1>
 * <1,0,0>
 * <1,0,1>
 * <1,1,0>
 * <1,1,1>
 * <1,2,0>
 * <1,2,1>
 * After construction, the Range object starts to point to the first element in
 * the joint domain <0,0,...>. Then, all other values can be iterated as with a
 * normal iterator.
 *
 * This object should be recognized by the compiler as an stl iterator.
 */
template <typename Support> class RangeT {
public:
  /** @param sizes of the variables this group represents
   */
  RangeT(Support &&sizes);

  std::optional<const Support *> next();

private:
  bool first_done_;

  struct Data {
    Support sizes;
    Support combination;
  };
  Data data_;
};

template <std::size_t N> using Range = RangeT<Combination<N>>;

using RangeVec = RangeT<std::vector<VarStateSize>>;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename Support>
RangeT<Support>::RangeT(Support &&sizes)
    : first_done_{false}, data_{std::forward<Support>(sizes), {}} {
  if constexpr (std::is_same_v<Support, std::vector<VarStateSize>>) {
    if (data_.sizes.empty()) {
      throw Error{"Ranges should not be built from empty sizes"};
    }

    data_.combination.reserve(data_.sizes.size());
    for (std::size_t k = 0; k < data_.sizes.size(); ++k) {
      data_.combination.push_back(0);
    }
  }
}

template <typename Support>
std::optional<const Support *> RangeT<Support>::next() {
  if (first_done_) [[likely]] {
    std::size_t k = data_.sizes.size() - 1;
    while (true) {
      if (data_.sizes[k] == ++data_.combination[k]) {
        if (k == 0) {
          return std::nullopt;
        } else {
          data_.combination[k] = 0;
          --k;
        }
      } else
        break;
    }
    return &data_.combination;
  } else {
    first_done_ = true;
    return &data_.combination;
  }
}

} // namespace EFG::categoric
