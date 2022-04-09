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
   * @brief exports the model (variables and factors) into an xml file
   * @param the model to export
   * @param the folder that will store the xml
   * @param the xml file name
   */
  template <typename Model>
  static nlohmann::json exportToJson(const Model &model,
                                     const std::string &file_path) {
    nlohmann::json result;
    convert(result, getAwareComponents(model));
    return result;
  };

  template <typename Model>
  static void exportToFile(const Model &model, const std::string &file_path) {
    auto as_json = exportToJson(model, file_path);
    auto stream = make_out_stream(file_path);
    *stream << as_json.dump();
  }

private:
  static void convert(nlohmann::json &recipient, const AwarePtrs &subject);
};
} // namespace EFG::io::json
#endif
