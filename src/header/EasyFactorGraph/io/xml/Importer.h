#ifdef EFG_XML_IO
/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/io/Importer.h>

namespace EFG::io::xml {
class Importer : public io::Importer {
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
  static std::unordered_set<std::string> importFromXml(Model &model,
                                                       const File &filePath) {
    return Importer().importComponents(filePath, getComponents(model));
  };

private:
  std::unordered_set<std::string>
  importComponents(const File &filePath, const AdderPtrs &subject) final;
};
} // namespace EFG::io::xml
#endif
