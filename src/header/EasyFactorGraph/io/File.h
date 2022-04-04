/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#include <string>

namespace EFG::io {
class File {
public:
  File(const std::string &path);

  const std::string &parent_str() const { return parent; };
  std::string str() const;

private:
  std::string parent;
  std::string file_name;
};
} // namespace EFG::io
