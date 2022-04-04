#ifdef EFG_XML_IO
/**
 * Author:    Andrea Casalino
 * Created:   01.01.2021
 *
 * report any bug to andrecasa91@gmail.com.
 **/

#pragma once

#include <EasyFactorGraph/io/Exporter.h>

namespace EFG::io::xml {
class Exporter : public io::Exporter {
public:
  /**
   * @brief exports the model (variables and factors) into an xml file
   * @param the model to export
   * @param the folder that will store the xml
   * @param the xml file name
   */
  template <typename Model>
  static void exportToXml(const Model &model, const std::string &filePath,
                          const std::string &modelName = "") {
    Exporter{}.exportComponents(filePath, modelName, getComponents(model));
  };

private:
  void exportComponents(const std::string &filePath,
                        const std::string &modelName,
                        const AwarePtrs &subject) final;
};
} // namespace EFG::io::xml
#endif
