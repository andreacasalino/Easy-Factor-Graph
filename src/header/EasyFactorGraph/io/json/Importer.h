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

#include <nlohmann/json.hpp>

namespace EFG::io::json {
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
   * @return the evidences contained in the parsed file
   */
  template <typename Model>
  static std::unordered_set<std::string> importFromFile(Model &model,
                                                        const File &file_path) {
    auto stream = make_in_stream(file_path.str());
    nlohmann::json source = nlohmann::json::parse(*stream);
    return convert(getAdderComponents(model), source);
  };

  /**
   * @brief parse the model (variables and factors) described by the passed
   * json and tries to add its factors to the passed model.
   * @param recipient of the model parsed from file
   * @param json describing the model to parse and add to the passed one
   * @return the evidences contained in the parsed file
   */
  template <typename Model>
  static std::unordered_set<std::string>
  importFromJson(Model &model, const nlohmann::json &source) {
    return convert(getAdderComponents(model), source);
  };

private:
  static std::unordered_set<std::string> convert(const AdderPtrs &recipient,
                                                 const nlohmann::json &source);
};
} // namespace EFG::io::json
#endif
