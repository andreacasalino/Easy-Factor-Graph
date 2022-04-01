/**
 * Author:    Andrea Casalino
 * Created:   23.05.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <string>

namespace EFG::io {
class FilePath {
public:
  FilePath(const std::string &fullPath);
  FilePath(const std::string &path, const std::string &fileName);

  inline const std::string &getPath() const { return this->path; };
  inline const std::string &getFileName() const { return this->fileName; };

  std::string getFullPath() const;

private:
  std::string path = ".";
  std::string fileName;
};
} // namespace EFG::io
