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
   * @brief imports the structure (variables and factors) described in
   an xml file and add it to the passed model
   * @param the model receiving the parsed data
   * @param the path storing the xml to import
   * @return the set of evidences red from the file. Attention! this
   quantities are returned to allow the user
   * to set such evidence, since this is not automatically done when
   importing
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
