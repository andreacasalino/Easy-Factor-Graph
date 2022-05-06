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
#include <EasyFactorGraph/structure/EvidenceManager.h>
#include <EasyFactorGraph/misc/DynamicPredicate.h>

namespace EFG::io::xml {
class Importer {
public:
  /**
   * @brief parse the model (variables and factors) described by the specified
   * file and tries to add its factors to the passed model.
   * @param recipient of the model parsed from file
   * @param location of the model to parse and add to the passed one
   */
  template <typename Model>
  static void importFromFile(Model &model, const File &file_path) {
    auto evidences = convert(getAdderComponents(model), file_path);
    dynamic_predicate<strct::EvidenceSetter>(model, [&evidences](strct::EvidenceSetter& as_setter) {
        for (const auto& [var, val] : evidences) {
            as_setter.setEvidence(as_setter.findVariable(var), val);
        }
    });
  }

private:
  static std::unordered_map<std::string, std::size_t>
  convert(const AdderPtrs &recipient, const File &file_path);
};
} // namespace EFG::io::xml
#endif
