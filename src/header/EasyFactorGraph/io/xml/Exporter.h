/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_XML_IO

#pragma once

#include <EasyFactorGraph/io/ModelComponents.h>

#include <filesystem>

namespace EFG::io::xml {
struct ExportInfo {
  std::string file_path;
  std::string model_name = "Model";
};

class Exporter {
public:
  /**
   * @brief exports the model (variables and factors) into a string,
   * describing an xml.
   * @param the model to export
   * @param the model name to report in the xml
   */
  template <typename Model>
  static std::string exportToString(const Model &model,
                                    const std::string &model_name) {
    std::stringstream stream;
    convert(stream, castToGetters(model), model_name);
    return stream.str();
  }

  /**
   * @brief exports the model (variables and factors) into an xml file
   * @param the model to export
   * @param info describing the xml to generate.
   */
  template <typename Model>
  static void exportToFile(const Model &model, const ExportInfo &info) {
    convert(info.file_path, castToGetters(model), info.model_name);
  }

private:
  static void convert(const std::filesystem::path &out_file, Getters subject,
                      const std::string &model_name);
};
} // namespace EFG::io::xml
#endif
