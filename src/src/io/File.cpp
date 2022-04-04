/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/Error.h>
#include <EasyFactorGraph/io/File.h>

#include <filesystem>
#include <sstream>

namespace EFG::io {
std::string File::str() const {
  std::stringstream stream;
  stream << parent << '/' << file_name;
  return stream.str();
}

File::File(const std::string &path) {
  auto absolute_path = std::filesystem::path(path);
  if (std::filesystem::is_directory(absolute_path)) {
    throw Error{
        "File object should be representative of file and not directory"};
  }
  parent = absolute_path.parent_path();
  file_name = absolute_path.filename();
}
} // namespace EFG::io
