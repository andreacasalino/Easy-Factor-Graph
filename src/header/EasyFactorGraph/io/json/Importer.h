/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#pragma once

#include <EasyFactorGraph/io/File.h>
#include <EasyFactorGraph/io/Utils.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>

#include <nlohmann/json.hpp>

namespace EFG::io::json {
class Importer {
public:
  /**
   * @brief imports the structure (variables and factors) described in
   an xml file and add it to the passed model
   * @param the model receiving the parsed data
   * @param the path storing the xml to import
   */
  template <typename Model>
  static void importFromFile(Model &model, const File &file_path) {
    auto stream = make_in_stream(file_path.str());
    nlohmann::json source = nlohmann::json::parse(*stream);
    auto evidences = convert(getAdderComponents(model), source);
    set_evidences(model, evidences);
  };

  /**
   * @brief parse the model (variables and factors) described by the passed
   * json and tries to add its factors to the passed model.
   * @param recipient of the model parsed from file
   * @param json describing the model to parse and add to the passed one
   */
  template <typename Model>
  static std::unordered_set<std::string>
  importFromJson(Model &model, const nlohmann::json &source) {
    auto evidences = convert(getAdderComponents(model), source);
    set_evidences(model, evidences);
  };

private:
  static std::unordered_set<std::string> convert(const AdderPtrs &recipient,
                                                 const nlohmann::json &source);

  template <typename Model>
  static void set_evidences(Model &model,
                            const std::unordered_set<std::string> &ev) {
    strct::EvidenceSetter *as_setter =
        dynamic_cast<strct::EvidenceSetter *>(&model);
    if (nullptr == as_setter) {
      return;
    }
    for (const auto &var : ev) {
      as_setter->setEvidence(as_setter->findVariable(var), 0);
    }
  }
};
} // namespace EFG::io::json
#endif
