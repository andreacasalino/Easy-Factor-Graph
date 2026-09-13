/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/Error.h>

#include <filesystem>
#include <fstream>
#include <memory>

namespace EFG::misc {
enum class FileMode { READ, WRITE };

template <FileMode M> struct ModeToStream;

template <> struct ModeToStream<FileMode::READ> {
  using Stream = std::ifstream;
};

template <> struct ModeToStream<FileMode::WRITE> {
  using Stream = std::ofstream;
};

template <FileMode M>
std::unique_ptr<typename ModeToStream<M>::Stream>
open(const std::filesystem::path &location) {
  if constexpr (M == FileMode::READ) {
    if (!std::filesystem::exists(location)) {
      throw Error{"Inexistent file location: `{}`", location.string()};
    }
  }

  using Stream = typename ModeToStream<M>::Stream;
  auto stream = std::make_unique<Stream>(location);
  if (!stream->is_open()) {
    throw Error{"Unable to open stream to: `{}`", location.string()};
  }
  return stream;
}

} // namespace EFG::misc
