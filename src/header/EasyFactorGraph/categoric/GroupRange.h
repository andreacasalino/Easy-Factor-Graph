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
 * the joint domain <0,0,...>. Then, when incrementing the object, the following
 * element is pointed. When calling get() the current pointed element can be
 * accessed.
 *
 * This object should be recognized by the compiler as an stl iterator.
 */
class GroupRange {
public:
  // https://www.internalpointers.com/post/writing-custom-iterators-modern-cpp
  using iterator_category = std::input_iterator_tag;
  // using difference_type = std::ptrdiff_t;
  using value_type = Combination;
  using pointer = Combination *;
  using reference = Combination &;

  /** @param the group of variables whose joint domain must be iterated
   */
  GroupRange(const Group &variables);

  GroupRange(const GroupRange &o);

  pointer operator->() const { return data->combination.get(); }
  reference operator*() const { return *data->combination.get(); }

  /**
   * @brief Make the object to point to the next element in the joint domain.
   * @throw if the current pointed element is the last one.
   */
  GroupRange &operator++();

  static GroupRange end() { return GroupRange{}; };

  bool isEqual(const GroupRange &o) const { return this->data == o.data; };

private:
  GroupRange() = default;

  struct Data {
    const std::vector<size_t> sizes;
    std::unique_ptr<Combination> combination;
    bool end_of_range;
  };
  std::unique_ptr<Data> data;
};

static const GroupRange RANGE_END = GroupRange::end();

bool operator==(const GroupRange &a, const GroupRange &b);
bool operator!=(const GroupRange &a, const GroupRange &b);

/**
 * @brief Applies the passed predicate to all the elements that can be ranged
 * using the passed range.
 */
template <typename Predicate>
void for_each_combination(GroupRange &range, const Predicate &predicate) {
  for (; range != RANGE_END; ++range) {
    predicate(*range);
  }
}
} // namespace EFG::categoric
