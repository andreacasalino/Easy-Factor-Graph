/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <sstream>
#include <stdexcept>

namespace EFG {
class MessagesMerger {
public:
  template <typename T1, typename T2, typename... Slices>
  static std::string merge(const T1 &first, const T2 &second,
                           const Slices &...args) {
    std::stringstream stream;
    merge_(stream, first, second, args...);
    return stream.str();
  }

private:
  template <typename T, typename... Slices>
  static void merge_(std::stringstream &recipient, const T &element,
                     const Slices &...args) {
    merge_(recipient, element);
    merge_(recipient, args...);
  }

  template <typename T>
  static void merge_(std::stringstream &recipient, const T &element) {
    recipient << element;
  }
};

class Error : public std::runtime_error {
public:
  Error(const std::string &what);

  template <typename T1, typename T2, typename... Slices>
  Error(const T1 &first, const T2 &second, const Slices &...args)
      : Error(MessagesMerger::merge(first, second, args...)) {}
};
} // namespace EFG
