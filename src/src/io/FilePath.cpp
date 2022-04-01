/**
 * Author:    Andrea Casalino
 * Created:   23.05.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <EasyFactorGraph/io/FilePath.h>

#include <sstream>

// constexpr char SEPARATOR = '/';
// #ifdef _WIN32
// constexpr char SEPARATOR2 = '\\';
// #endif

namespace EFG::io {
// FilePath::FilePath(const std::string &path, const std::string &fileName)
//     : path(path), fileName(fileName) {}

// #ifdef _WIN32
//     std::string getFixedFullPath(const std::string& fullPath) {
//         std::string fullPathFixed = fullPath;
//         // fix all \\ into /
//         for (std::size_t k = 0; k < fullPathFixed.size(); ++k) {
//             if (fullPathFixed[k] == SEPARATOR2) {
//                 fullPathFixed[k] = SEPARATOR;
//             }
//         }
//         return fullPathFixed;
//     }
// #endif

//     FilePath::FilePath(const std::string& fullPath) {
// #ifdef _WIN32
//         std::string fullPathFixed = getFixedFullPath(fullPath);
// #else
//         std::string fullPathFixed = fullPath;
// #endif
//         // find last /
//         for (std::size_t k = fullPathFixed.size() - 2; k > 0; --k) {
//             if (SEPARATOR == fullPathFixed[k]) {
//                 this->path = std::string(fullPathFixed, 0, k+1);
//                 this->fileName = std::string(fullPathFixed, k+1,
//                 fullPathFixed.size() - k - 1); return;
//             }
//         }
//         this->fileName = fullPathFixed;
//     }

std::string FilePath::getFullPath() const {
  std::stringstream stream;
  stream << this->path << '/' << this->fileName;
  return stream.str();
}
} // namespace EFG::io
