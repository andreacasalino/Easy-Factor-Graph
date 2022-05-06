/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#pragma once

#include <EasyFactorGraph/io/Utils.h>

#include <nlohmann/json.hpp>

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
    convert(result, getAwareComponents(model));
    return result;
  }

  /**
   * @brief exports the model (variables and factors) into an json file
   * @param the model to export
   * @param the file to generate storing the exported json
   */
  template <typename Model>
  static void exportToFile(const Model &model, const std::string &file_path) {
    auto as_json = exportToJson(model);
    auto stream = make_out_stream(file_path);
    *stream << as_json.dump();
  }

private:
  static void convert(nlohmann::json &recipient, const AwarePtrs &subject);
};
} // namespace EFG::io::json
#endif
