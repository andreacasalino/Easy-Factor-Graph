/**
 * Author:    Andrea Casalino
 * Created:   31.03.2022
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/misc/MemoryPool.h>

namespace EFG::misc {
MemoryPool::MemoryPool(std::size_t capacity_limit)
    : len_{0}, capacity_{capacity_limit}, buffer_{new char[capacity_limit]} {}

NumbersPoolSizes::NumbersPoolSizes() = default;
} // namespace EFG::misc
