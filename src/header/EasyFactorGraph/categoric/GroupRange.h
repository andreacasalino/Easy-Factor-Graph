/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Combination.h>
#include <EasyFactorGraph/categoric/Group.h>

namespace EFG::categoric {
/**
 * @brief This object allows you to iterate all the elements in the joint domain
 * of a group of variables, without precomputing all the elements in such
 * domain. For example when having a domain made by variables = { A (size = 2),
 * B (size = 3), C (size  = 2)  }, the elements in the joint domain that will be
 * iterated are: <0,0,0> <0,0,1> <0,1,0> <0,1,1> <0,2,0> <0,2,1> <1,0,0> <1,0,1>
 * <1,1,0>
 * <1,1,1>
 * <1,2,0>
 * <1,2,1>
 * After construction, the Range object starts to point to the first element in
 * the joint domain <0,0,...>. Then, when incrementing the object, the following
 * element is pointed. When calling get() the current pointed element can be
 * accessed.
 */
class Range {
public:
  // https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
  using iterator_category = std::input_iterator_tag;
  // using difference_type = std::ptrdiff_t;
  using value_type = Combination;
  using pointer = Combination *;
  using reference = Combination &;

  /** @param the group of variables whose joint domain must be iterated
   */
  explicit Range(const Group &variables);

  Range() = default;

  /**
   * @brief Make the object to point to the first element of the joint domain
   * <0,0,...>, i.e. reset the status as it is after construction.
   */
  Range reset();

  const pointer operator->() const { return &data->combination; }
  const reference operator*() const { return data->combination; }

  /**
   * @brief Make the object to point to the next element in the joint domain.
   * @throw if the current pointed element is the last one.
   */
  Range &operator++();

  static Range end() { return Range{}; };

  bool isEqual(const Range &o) const;

private:
  struct Data {
    const std::vector<size_t> sizes;
    Combination combination;
  };
  std::unique_ptr<Data> data;
};

constexpr Range RangeEnd;

bool operator==(const Range &a, const Range &b) { return a.isEqual(b); };
bool operator!=(const Range &a, const Range &b) { return !a.isEqual(b); };

template <typename Predicate>
void for_each_combination(Range &range, const Predicate &predicate) {
  for (; range != RangeEnd; ++range) {
    predicate(*range);
  }
}
} // namespace EFG::categoric
