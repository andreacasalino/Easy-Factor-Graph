/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#ifdef EFG_JSON_IO

#pragma once

#include <EasyFactorGraph/io/Exporter.h>

namespace EFG::io::json {
class Exporter : public io::Exporter {
public:
  /**
   * @brief exports the model (variables and factors) into an xml file
   * @param the model to export
   * @param the folder that will store the xml
   * @param the xml file name
   */
  template <typename Model>
  static void exportToJson(const Model &model, const std::string &filePath,
                           const std::string &modelName = "") {
    Exporter{}.exportComponents(filePath, modelName, getComponents(model));
  };

private:
  void exportComponents(const File &filePath, const std::string &model_name,
                        const AwarePtrs &subject) final;
};
} // namespace EFG::io::json
#endif
