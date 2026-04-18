/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/factor/FactorT.h>

#include <ostream>
#include <span>
#include <vector>

template <typename T>
std::ostream &operator<<(std::ostream &s, std::span<const T> values) {
  if (values.empty()) {
    return s;
  }
  s << values[0];
  std::for_each(values.begin() + 1, values.end(), [&s](auto val) {
    s << ' ';
    s << val;
  });
  return s;
}

template <typename T>
std::ostream &operator<<(std::ostream &s, const std::vector<T> &values) {
  return s << std::span<const T>{values.begin(), values.end()};
}

class Tabular {
public:
  Tabular() = default;

  struct Line {
    template <typename... Args> Line(Args &&...args) {
      (add(std::move(args)), ...);
    }

    std::string_view get(std::size_t idx) const;

    std::string buffer_;
    std::vector<std::size_t> sizes;

  private:
    template <typename T> void add(T to_add) {
      if constexpr (std::is_same_v<T, std::string> ||
                    std::is_same_v<T, std::string_view>) {
        buffer_ += to_add;
        sizes.push_back(to_add.size());
      } else if constexpr (std::is_same_v<T, const char *>) {
        add(std::string_view{to_add});
      } else {
        add(std::to_string(to_add));
      }
    }
  };

  void addLine(Line line);

  void print(std::ostream &s) const;

private:
  std::vector<std::size_t> columns_max_len;
  std::vector<Line> lines;
};

template <std::size_t N, typename Transform>
std::ostream &print(std::ostream &s,
                    const EFG::factor::FactorT<N, Transform> &distribution);
