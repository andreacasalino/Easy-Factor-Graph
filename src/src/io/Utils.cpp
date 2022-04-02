/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include "Utils.h"
#include <EasyFactorGraph/Error.h>

namespace EFG::io {
using IStream = std::unique_ptr<std::ifstream>;

IStream make_stream(const std::string &file_name) {
  IStream stream = std::make_unique<std::ifstream>(file_name);
  if (!stream->is_open()) {
    throw Error{file_name, " is a non valid file"};
  }
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
