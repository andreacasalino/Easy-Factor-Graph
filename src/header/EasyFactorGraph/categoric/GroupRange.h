/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/categoric/Group.h>

#include <optional>

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
  using value_type = std::vector<std::size_t>;
  using pointer = const std::vector<std::size_t> *;
  using reference = const std::vector<std::size_t> &;

  GroupRange(const std::vector<std::size_t> &sizes);

  /** @param the group of variables whose joint domain must be iterated
   */
  GroupRange(const Group &variables);

  GroupRange(const GroupRange &o);

  pointer operator->() const { return &data->combination; }
  reference operator*() const { return data->combination; }

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
    Data(const std::vector<size_t> &s, bool eor);

    const std::vector<size_t> sizes;
    std::vector<std::size_t> combination;
    bool end_of_range;

    bool operator==(const Data &o) const {
      return (end_of_range == o.end_of_range) && (combination == o.combination);
    }
  };
  std::optional<Data> data;
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
