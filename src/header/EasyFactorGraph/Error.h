/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/misc/Strings.h>

#include <stdexcept>

namespace EFG {
class Error : public std::runtime_error {
public:
  Error(const std::string &what);

  template <typename... Args> static Error make(Args &&...args) {
    return Error{join<' '>(std::forward<Args>(args)...)};
  }
};
} // namespace EFG
