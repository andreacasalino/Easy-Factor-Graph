/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <stdexcept>
#include <string_view>

#include <EasyFactorGraph/misc/Format.h>

namespace EFG {
class Error : public std::runtime_error {
public:
  Error(std::string what);

  template <typename ArgFirst, typename... Args>
  Error(std::string_view frmt, ArgFirst &&first, Args &&...args)
      : Error{misc::format(frmt, std::forward<ArgFirst>(first),
                           std::forward<Args>(args)...)} {}
};
} // namespace EFG
