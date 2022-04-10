/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_XML_IO

#pragma once

#include <EasyFactorGraph/io/File.h>
#include <EasyFactorGraph/io/Utils.h>

namespace EFG::io::xml {
class Importer {
public:
  /**
   * @brief parse the model (variables and factors) described by the specified
   * file and tries to add its factors to the passed model.
   * @param recipient of the model parsed from file
   * @param location of the model to parse and add to the passed one
   * @return the evidences contained in the parsed file
   */
  template <typename Model>
  static std::unordered_set<std::string> importFromFile(Model &model,
                                                        const File &file_path) {
    return convert(getAdderComponents(model), file_path);
  };

private:
  static std::unordered_set<std::string> convert(const AdderPtrs &recipient,
                                                 const File &file_path);
};
} // namespace EFG::io::xml
#endif
