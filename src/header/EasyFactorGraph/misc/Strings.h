/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <sstream>
#include <string.h>

namespace EFG {
namespace detail {
template <char Separator> struct Merger {
  template <typename Last> void join_(Last &&last) { recipient << last; }

  template <typename First, typename... Args>
  void join_(First &&first, Args &&...args) {
    recipient << Separator << first;
    join_(std::forward<Args>(args)...);
  }

  std::ostream &recipient;
};

} // namespace detail

template <char Separator, typename... Args>
void join(std::ostream &recipient, Args &&...args) {
  detail::Merger<Separator> merger{recipient};
  merger.join_(std::forward<Args>(args)...);
}

template <char Separator, typename... Args> std::string join(Args &&...args) {
  std::stringstream buff;
  detail::Merger<Separator> merger{buff};
  merger.join_(std::forward<Args>(args)...);
  return buff.str();
}
} // namespace EFG
