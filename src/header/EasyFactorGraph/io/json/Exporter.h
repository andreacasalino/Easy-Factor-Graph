/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#pragma once

#include <EasyFactorGraph/io/ModelComponents.h>

#include <nlohmann/json.hpp>

#include <filesystem>

namespace EFG::io::json {
class Exporter {
public:
  /**
   * @brief exports the model (variables and factors) into a json.
   * @param the model to export
   */
  template <typename Model>
  static nlohmann::json exportToJson(const Model &model) {
    nlohmann::json result;
    convert(result, castToGetters(model));
    return result;
  }

  /**
   * @brief exports the model (variables and factors) into an json file
   * @param the model to export
   * @param the file to generate storing the exported json
   */
  template <typename Model>
  static void exportToFile(const Model &model,
                           const std::filesystem::path &file_path) {
    auto as_json = exportToJson(model);
    exportToFile(as_json, file_path);
  }

private:
  static void convert(nlohmann::json &recipient, Getters subject);

  static void exportToFile(const nlohmann::json &source,
                           const std::filesystem::path &out);
};
} // namespace EFG::io::json
#endif
