/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/io/Utils.h>

namespace EFG::io {
namespace {
template <typename T>
void check_stream(const T &stream, const std::string &file_name) {
  if (!stream->is_open()) {
    throw Error{file_name, " is a non valid file"};
  }
}
} // namespace

IStream make_in_stream(const std::string &file_name) {
  IStream stream = std::make_unique<std::ifstream>(file_name);
  check_stream(stream, file_name);
  return stream;
}

OStream make_out_stream(const std::string &file_name) {
  OStream stream = std::make_unique<std::ofstream>(file_name);
  check_stream(stream, file_name);
  return stream;
}

categoric::Combination
parse_combination(const std::vector<std::string> &values) {
  std::vector<std::size_t> temp;
  temp.reserve(values.size());
  for (const auto &value : values) {
    temp.push_back(static_cast<std::size_t>(std::atoi(value.c_str())));
  }
  return categoric::Combination{std::move(temp)};
}
} // namespace EFG::io
