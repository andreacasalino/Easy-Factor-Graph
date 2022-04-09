/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_XML_IO

#pragma once

#include <EasyFactorGraph/io/Utils.h>

#include <sstream>

namespace EFG::io::xml {
struct ExportInfo {
  std::string file_path;
  std::string model_name = "Model";
};

class Exporter {
public:
  /**
   * @brief exports the model (variables and factors) into an xml file
   * @param the model to export
   * @param the folder that will store the xml
   * @param the xml file name
   */
  template <typename Model>
  static std::string exportToString(const Model &model,
                                    const ExportInfo &info) {
    std::stringstream stream;
    convert(stream, getAwareComponents(model), info);
    return stream.str();
  };

  template <typename Model>
  static void exportToFile(const Model &model, const ExportInfo &info) {
    auto stream = make_out_stream(info.file_path);
    convert(*stream, getAwareComponents(model), info);
  }

private:
  static void convert(std::ostream &recipient, const AwarePtrs &subject,
                      const ExportInfo &info);
};
} // namespace EFG::io::xml
#endif
