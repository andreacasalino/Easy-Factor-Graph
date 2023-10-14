/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#pragma once

#include <EasyFactorGraph/io/ModelComponents.h>
#include <EasyFactorGraph/misc/Cast.h>
#include <EasyFactorGraph/structure/EvidenceManager.h>

#include <nlohmann/json.hpp>

#include <filesystem>

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
  static void importFromFile(Model &model,
                             const std::filesystem::path &file_path) {
    auto asJson = importJsonFromFile(file_path);
    importFromJson(model, asJson);
  }

  /**
   * @brief parse the model (variables and factors) described by the passed
   * json and tries to add its factors to the passed model.
   * @param recipient of the model parsed from file
   * @param json describing the model to parse and add to the passed one
   */
  template <typename Model>
  static void importFromJson(Model &model, const nlohmann::json &source) {
    auto evidences = convert(castToInserters(model), source);
    set_evidences(model, evidences);
  }

private:
  static std::unordered_map<std::string, std::size_t>
  convert(Inserters recipient, const nlohmann::json &source);

  static nlohmann::json
  importJsonFromFile(const std::filesystem::path &file_path);

  template <typename Model>
  static void
  set_evidences(Model &model,
                const std::unordered_map<std::string, std::size_t> &ev) {
    castAndUse<strct::EvidenceSetter>(
        model, [&ev](strct::EvidenceSetter &as_setter) {
          for (const auto &[var, val] : ev) {
            as_setter.setEvidence(as_setter.findVariable(var), val);
          }
        });
  }
};
} // namespace EFG::io::json
#endif
