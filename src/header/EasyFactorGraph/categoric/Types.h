/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <array>
#include <cstdint>

namespace EFG::categoric {
using VarStateSize = std::uint16_t;

template <std::size_t N> using Combination = std::array<std::uint16_t, N>;
} // namespace EFG::categoric
