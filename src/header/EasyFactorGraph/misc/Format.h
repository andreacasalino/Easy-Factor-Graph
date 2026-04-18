/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace EFG::misc {
template <typename ArgFirst, typename... Args>
std::string format(std::string_view frmt, ArgFirst &&first, Args &&...args) {
  std::string result;

  auto replace_next_ = [&](auto next_arg) {
    auto next_place_holder = frmt.find("{}");
    if (next_place_holder == std::string::npos) {
      throw std::runtime_error{"Invalid format"};
    }

    result += std::string_view{frmt.begin(), frmt.begin() + next_place_holder};
    if constexpr (std::is_same_v<std::string, decltype(next_arg)> ||
                  std::is_same_v<std::string_view, decltype(next_arg)>) {
      result += next_arg;
    } else {
      result += std::to_string(next_arg);
    }
    frmt = std::string_view{frmt.begin() + next_place_holder + 2, frmt.end()};
  };

  replace_next_(std::forward<ArgFirst>(first));
  (replace_next_(std::forward<Args>(args)), ...);

  return result;
}
} // namespace EFG::misc
